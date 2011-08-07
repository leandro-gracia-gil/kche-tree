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
 * \file benchmark_tool.tpp
 * \brief Generic benchmark tool template implementation.
 * \author Leandro Graciá Gil
 */

// C Standard Library and C++ STL includes.
#include <ctime>
#include <iostream>
#include <iomanip>
#include <vector>

/**
 * Create a new benchmark tool using the options provided in the command line.
 * Data sets are automatically loaded or generated.
 *
 * \tparam RandomEngineType Type of the random number generation engine used.
 * \param argc Number of params in command line.
 * \param argv Params of command line.
 * \param random_engine Random number generator used to initialize the contents of the train and test sets if required by the arguments.
 */
template <typename T, const unsigned int D> template <typename RandomEngineType>
BenchmarkTool<T, D>::BenchmarkTool(int argc, char *argv[], RandomEngineType &random_engine)
    : ToolBase<T, D, BenchmarkOptions>(argc, argv, random_engine) {}

/// Validate the values from the provided options.
template <typename T, const unsigned int D>
bool BenchmarkTool<T, D>::validate_options() const {

  // Validate the base options from the parent class.
  if (!ToolBase<T, D, BenchmarkOptions>::validate_options())
    return false;

  // Validate benchmark toll specific options.
  if (this->options_->knn_arg < 0) {
    std::cerr << "Invalid knn value." << std::endl;
    return false;
  }

  if (this->options_->bucket_size_arg <= 0) {
    std::cerr << "Invalid bucket size." << std::endl;
    return false;
  }

  if (this->options_->epsilon_arg < 0.0f) {
    std::cerr << "Invalid epsilon value. Should be 0 or greater." << std::endl;
    return false;
  }

  return true;
}

/**
 * \brief Run the benchmarking tool.
 *
 * Will measure the time required to build the tree and search its k-nearest neighbours
 * for each test sample according to the provided options.
 *
 * \tparam MetricType Type of the metric being used during the test.
 * \param metric Metric object to be used during the test.
 */
template <typename T, const unsigned int D> template <typename MetricType>
bool BenchmarkTool<T, D>::run(const MetricType &metric) {

  // Check if the tool is ready.
  if (!this->is_ready())
    return false;

  // Use the kche_tree namespace locally for simplicity.
  using namespace kche_tree;

  // Build the kd-tree.
  clock_t t1_build = clock();
  KDTree<T, D> kdtree;
  kdtree.build(this->train_set_, this->options_->bucket_size_arg);
  clock_t t2_build = clock();

  // Process each test case.
  clock_t t1_test = clock();
  for (unsigned int i=0; i < this->test_set_.size(); ++i) {

    // Get the K nearest neighbours.
    std::vector<typename KDTree<T, D>::NeighbourType> knn;
    if (this->options_->use_k_heap_flag)
      kdtree.template knn<KHeap>(this->test_set_[i], this->options_->knn_arg, knn, metric, this->options_->epsilon_arg, this->options_->ignore_existing_flag);
    else
      kdtree.template knn<KVector>(this->test_set_[i], this->options_->knn_arg, knn, metric, this->options_->epsilon_arg, this->options_->ignore_existing_flag);
  }
  clock_t t2_test = clock();

  // Calculate times.
  double time_build = (t2_build - t1_build) / static_cast<double>(CLOCKS_PER_SEC);
  double time_test  = (t2_test  - t1_test) / static_cast<double>(CLOCKS_PER_SEC);
  double build_percent = 100.0 * time_build / (time_build + time_test);
  double test_percent  = 100.0 * time_test  / (time_build + time_test);
  double test_average  = time_test / static_cast<double>(this->test_set_.size());

  // Report results.
  std::cout << std::fixed;
  std::cout << "Kd-tree testing with D = " << D << ", " << this->options_->knn_arg
      << " neighbours, epsilon " << std::setprecision(2) << this->options_->epsilon_arg
      << ", training set size " << this->train_set_.size() << ", test set size " << this->test_set_.size() << std::endl;
  std::cout << "Build time: " << std::setprecision(3) << time_build << " sec (" << std::setprecision(2) << build_percent << "%)" << std::endl;
  std::cout << "Test  time: " << std::setprecision(3) << time_test  << " sec (" << std::setprecision(2) << test_percent  << "%) -- average per test: "
      << std::setprecision(4) << test_average << " sec" << std::endl;
  std::cout << "Total time: " << std::setprecision(3) << (time_build + time_test) << " sec" << std::endl;

  return true;
}
