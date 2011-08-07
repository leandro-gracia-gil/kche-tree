/***************************************************************************
 *   Copyright (C) 2010, 2011 by Leandro Graciá Gil                        *
 *   leandro.gracia.gil@gmail.com                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/**
 * \file verification_tool.tpp
 * \brief Generic verification tool template implementation.
 * \author Leandro Graciá Gil
 */

// Static data.
template <typename T, const unsigned int D>
const char VerificationTool<T, D>::serialization_test_filename[] = "kd-tree.test";

/**
 * \brief Create a new verification tool using the options provided in the command line.
 * Data sets are automatically loaded or generated.
 *
 * \tparam RandomEngineType Type of the random number generation engine used.
 * \param argc Number of params in command line.
 * \param argv Params of command line.
 * \param random_engine Random number generator used to initialize the contents of the train and test sets if required by the arguments.
 */
template <typename T, const unsigned int D> template <typename RandomEngineType>
VerificationTool<T, D>::VerificationTool(int argc, char *argv[], RandomEngineType &random_engine)
    : ToolBase<T, D, VerificationOptions>(argc, argv, random_engine) {}

/// Validate the values from the provided options.
template <typename T, const unsigned int D>
bool VerificationTool<T, D>::validate_options() const {

  if (!ToolBase<T, D, VerificationOptions>::validate_options())
    return false;

  if (this->options_->knn_arg < 0) {
    std::cerr << "Invalid knn value." << std::endl;
    return false;
  }

  if (this->options_->all_in_range_arg < 0) {
    std::cerr << "Invalid all-in-range value." << std::endl;
    return false;
  }

  if (this->options_->bucket_size_arg <= 0) {
    std::cerr << "Invalid bucket size." << std::endl;
    return false;
  }

  if (this->options_->tolerance_arg < 0.0f) {
    std::cerr << "Invalid tolerance value." << std::endl;
    return false;
  }

  if (this->options_->epsilon_arg < 0.0f) {
    std::cerr << "Invalid epsilon value. Should be 0 or greater." << std::endl;
    return false;
  }

  return true;
}

/**
 * \brief Run the verification tool.
 *
 * Will compare the k-nearest neighbours and the all-in-range results with
 * an exhaustive all to all search according to the provided options.
 * Will print information about any errors found during the process.
 *
 * \tparam MetricType Type of the metric being used during the test.
 * \param metric Metric object to be used during the test.
 */
template <typename T, const unsigned int D> template <typename MetricType>
bool VerificationTool<T, D>::run(const MetricType &metric) {

  // Check if the tool is ready.
  if (!this->is_ready())
    return false;

  // Use the kche_tree namespace locally for simplicity.
  using namespace kche_tree;

  // Define the global result flag.
  bool ok = true;

  // Build the kd-tree.
  typedef KDTree<T, D> KDTreeTool;
  KDTreeTool kdtree;
  kdtree.build(this->train_set_, this->options_->bucket_size_arg);

  // Test the kd-tree I/O.
  if (this->options_->kdtree_io_flag) {

    // Try to save the kd-tree to a file.
    std::ofstream out_file(serialization_test_filename, std::ios::binary | std::ios::out);
    out_file << kdtree;
    out_file.close();

    // Read the tree back.
    std::ifstream in_file(serialization_test_filename, std::ios::binary | std::ios::in);
    in_file >> kdtree;
    in_file.close();
  }

  // Test the subscript operator.
  if (this->options_->subscript_flag) {
    for (unsigned int i=0; i<this->train_set_.size(); ++i) {
      if (this->train_set_[i] != kdtree[i]) {
        std::cerr << "Non-matching subscript operator value for index " << i << "." << std::endl;
        ok = false;
      }
    }
  }

  // Allocate memory for the exhaustive calculation of nearest neighbours.
  typedef typename KDTreeTool::NeighbourType NeighbourType;
  ScopedArray<NeighbourType> nearest(new NeighbourType[this->train_set_.size()]);
  assert(nearest);

  // Process each test case.
  for (unsigned int i=0; i<this->test_set_.size(); ++i) {

    // Create a vector of point-distance tuples to current test point.
    if (this->options_->knn_arg > 0 || this->options_->all_in_range_arg > 0.0f) {
      for (unsigned int n=0; n<this->train_set_.size(); ++n) {
        T distance = metric(this->train_set_[n], this->test_set_[i]);

        // Check numerical consistency of the distance.
        if ((distance == Traits<T>::zero() && this->test_set_[i] != this->train_set_[n]) || (!(distance == Traits<T>::zero()) && this->test_set_[i] == this->train_set_[n]))
          std::cerr << "Warning: possible numerical precision problem. Distance from a point to itself not strictly zero." << std::endl;

        // Exclude the point from the nearest neighbours as knn will do.
        if (this->options_->ignore_existing_flag && distance == Traits<T>::zero())
          distance = Traits<T>::max();

        nearest[n] = NeighbourType(n, distance);
      }
    }

    // Sort the training set vectors by its distance to the test point.
    std::sort(nearest.get(), nearest.get() + this->train_set_.size(), nearest[0]);

    // Test the knn functionality.
    if (this->options_->knn_arg > 0) {

      // Get the K nearest neighbours.
      unsigned int K = this->options_->knn_arg;
      std::vector<typename KDTreeTool::NeighbourType> knn;
      if (this->options_->use_k_heap_flag)
        kdtree.template knn<KHeap>(this->test_set_[i], K, knn, metric, T(this->options_->epsilon_arg), this->options_->ignore_existing_flag);
      else
        kdtree.template knn<KVector>(this->test_set_[i], K, knn, metric, T(this->options_->epsilon_arg), this->options_->ignore_existing_flag);

      // Check the k nearest neighbours returned.
      unsigned int num_elems = std::min(static_cast<unsigned int>(knn.size()), K);
      for (unsigned int k=0; k<num_elems; ++k) {

        // Check if the points in the tree were correctly ignored if requested.
        if (this->options_->ignore_existing_flag && this->test_set_[i] == this->train_set_[knn[k].index] &&
            !(nearest[knn[k].index].squared_distance == Traits<T>::max())) {
          std::cerr << "Nearest neighbour " << k << " failed (in the tree but not ignored): index " << nearest[k].index
              << "(" << nearest[k].squared_distance << "), expected index " << knn[k].index << " (" << knn[k].squared_distance
              << ") in test case " << i << std::endl;
          ok = false;
        }

        // Check if distances match with expected ones.
        T difference = knn[k].squared_distance;
        difference -= nearest[k].squared_distance;
        Traits<T>::abs(difference);
        if (difference > T(this->options_->tolerance_arg)) {
          std::cerr << "Nearest neighbour " << k << " failed: index " << nearest[k].index << " (" <<nearest[k].squared_distance
              << "), expected index " << knn[k].index << " (" << knn[k].squared_distance << ") in test case " << i << std::endl;
          ok = false;
        }

        // Check if distance calculations are correct within a tolerance value.
        // Since the knn method uses incremental calculations, depending on the type used the result
        // values can be slightly different. This can be more accute with higher dimension values.
        T dist1 = metric(this->train_set_[knn[k].index], this->test_set_[i]);
        T dist2 = nearest[k].squared_distance;

        difference = dist1;
        difference -= dist2;
        Traits<T>::abs(difference);

        T sqr_tolerance(this->options_->tolerance_arg);
        sqr_tolerance *= sqr_tolerance;

        if (difference > sqr_tolerance) {
          std::cerr << "Nearest neighbour " << k << " failed: returned distance doesn't match (" << dist1
              << " != " << dist2 << ") in test case " << i << std::endl;
          ok = false;
        }
      }

      // Check vector size.
      if (knn.size() != num_elems) {
        std::cerr << "Wrong nearest neighbour vector size (" << knn.size() << ", expected " << num_elems << ") in test case " << i << std::endl;
        ok = false;
      }
    }

    // Test the all-in-range functionality.
    if (this->options_->all_in_range_arg > 0.0f) {

      // Calculate the squared range search distance.
      T search_range = this->options_->all_in_range_arg;
      T squared_search_range = search_range;
      squared_search_range *= squared_search_range;

      T sqr_tolerance(this->options_->tolerance_arg);
      sqr_tolerance *= sqr_tolerance;

      T tolerance_range = squared_search_range;
      tolerance_range += sqr_tolerance;

      // Get all the neighbours in a random range.
      std::vector<NeighbourType> points_in_range;
      kdtree.all_in_range(this->test_set_[i], search_range, points_in_range, metric, this->options_->ignore_existing_flag);

      // Check the returned neighbours within the range.
      for (unsigned int k=0; k<points_in_range.size(); ++k) {

        // Check if the points in the tree were correctly ignored if requested.
        if (this->options_->ignore_existing_flag && this->test_set_[i] == this->train_set_[points_in_range[k].index] &&
            !(nearest[points_in_range[k].index].squared_distance == Traits<T>::max())) {
          std::cerr << "In-range point: index " << points_in_range[k].index << " ("
              << points_in_range[i].squared_distance << ") in tree but not ignored in test case " << i << std::endl;
          ok = false;
        }

        // Check point lies within the range.
        if (points_in_range[k].squared_distance > tolerance_range) {
          std::cerr << "In-range point failed: index " << points_in_range[k].index << " (" << points_in_range[i].squared_distance <<
              ") greater than requested squared distance " << squared_search_range << " in test case " << i << std::endl;
          ok = false;
        }

        // Check if returned distances were properly calculated.
        T dist1 = metric(this->train_set_[points_in_range[k].index], this->test_set_[i]);
        T dist2 = points_in_range[k].squared_distance;

        T difference = dist1;
        difference -= dist2;
        Traits<T>::abs(difference);

        if (difference > sqr_tolerance) {
          std::cerr << "In-range point failed: returned squared distance doesnt't match (" << dist1 <<
              " != " << dist2 << ") in test case " << i << std::endl;
          ok = false;
        }
      }

      // Count number of points in range.
      unsigned int in_range = 0;
      for (unsigned int n=0; n<this->train_set_.size(); ++n) {
        if (!(nearest[n].squared_distance > squared_search_range)) {
          assert(!(this->options_->ignore_existing_flag && nearest[n].squared_distance == Traits<T>::zero()));
          ++in_range;
        }
      }

      // Check number of points in range.
      if (in_range != points_in_range.size()) {
        std::cerr << "Wrong number of neighbours within range " << this->options_->all_in_range_arg << " (found " <<
            points_in_range.size() << ", expected " << in_range << ") in test case " << i << std::endl;
        ok = false;
      }
    }
  }

  // Report results.
  if (ok)
    std::cout << "All tests OK!" << std::endl;
  else
    std::cout << "Problems found during testing..." << std::endl;

  return ok;
}

