/***************************************************************************
 *   Copyright (C) 2010, 2011, 2012 by Leandro Graciá Gil                  *
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
 * \file kd-tree.tpp
 * \brief Template implementations for generic kd-trees.
 * \author Leandro Graciá Gil
 */

namespace kche_tree {

// Include the out_of_range exception.
#include <stdexcept>

/**
 * \brief Creates an empty, uninitialized kd-tree.
 */
template <typename T, unsigned int D, typename L>
KDTree<T, D, L>::KDTree() : data_(new DataSet()) {}

/**
 * \brief Get the data contained by the kd-tree.
 */
template <typename T, unsigned int D, typename L>
const typename KDTree<T, D, L>::DataSet& KDTree<T, D, L>::data() const {
  KCHE_TREE_DCHECK(data_);
  return *data_;
}

/**
 * \brief Get the number of elements stored in the tree.
 */
template <typename T, unsigned int D, typename L>
unsigned int KDTree<T, D, L>::size() const {
  KCHE_TREE_DCHECK(data_);
  return data_->size();
}

/**
 * \brief Convenience constructor to build a kd-tree directly from a training set.
 */
template <typename T, unsigned int D, typename L>
KDTree<T, D, L>::KDTree(const DataSet &train_set, unsigned int bucket_size) {
  if (!build(train_set, bucket_size))
    data_.reset(new DataSet());
}

/**
 * Build a kd-tree from a set of \a n D-dimensional samples.
 *
 * \param train_set Train set used to build the kd-tree.
 * \param bucket_size Number of elements that should be grouped in leaf nodes.
 * \return \c true if successful, \c false otherwise.
 */
template <typename T, unsigned int D, typename L>
bool KDTree<T, D, L>::build(const DataSet &train_set, unsigned int bucket_size) {

  // Check params.
  unsigned int num_points = train_set.size();
  if (num_points == 0 || bucket_size == 0)
    return false;

  // Allocate and initialize the permutation array to identity.
  ScopedArray<unsigned int> permutation(new unsigned int[num_points]);
  for (unsigned int i=0; i<num_points; ++i)
    permutation[i] = i;

  // Build the kd-tree recursively (num_elements will contain a recursively-calculated num_points after the call).
  unsigned int num_elements = 0;
  root_.reset(KDNode::build(train_set, permutation.get(), train_set.size(), NULL, bucket_size, num_elements));
  KCHE_TREE_DCHECK(num_elements == num_points);

  // Make a local permuted copy of the train data. The permutation vector ownership is transferred to the data set.
  data_.reset(new DataSet(train_set, permutation.release()));

  return true;
}

/**
 * Find the K nearest neighbors of a given Point and push their indices sorted into a given STL vector.
 * In case that there are not enough points in the tree, all the available ones will be provided.
 *
 * \param p Point whose \a K neighbors should be retrieved.
 * \param K Number of nearest neighbors to retrieve.
 * \param output STL vector where the nearest neighbors will be appended. Sorted result depends on the template parameter S, enabled by default.
 * \param metric Metric functor that will be used to calculate the distances between points. Defaults to Euclidean metric if C++1x is enabled.
 * \param epsilon Acceptable distance margin to ignore regions during kd-tree exploration. Defaults to zero (deterministic).
 * \param ignore_p_in_tree Assume that \a p is contained in the tree any number of times and ignore them all.
 */
template <typename T, unsigned int D, typename L> template <template <typename, typename> class KContainer, typename Metric>
void KDTree<T, D, L>::knn(const Vector &p, unsigned int K, std::vector<Neighbor> &output, const Metric &metric, ConstRef_Distance epsilon, bool ignore_p_in_tree) const {
  // Check if there is any data on the tree and K is valid.
  KCHE_TREE_DCHECK(data_);
  if (!root_ || size() == 0 || K == 0)
    return;

  // Create an object for tree traversal and incremental hyperrectangle intersection calculation.
  KDSearch<Element, Dimensions, Metric> search_data(p, *data_, metric, K, ignore_p_in_tree);

  // Convert epsilon to a squared distance and set it as initial hyperrectangle distance.
  search_data.hyperrect_distance = epsilon;
  search_data.hyperrect_distance *= epsilon;

  // Build a special sorted container for the current K nearest neighbor candidates.
  KContainer<Neighbor, typename Neighbor::DistanceComparer> best_k(K);

  // Start an exploration traversal from the root.
  root_->explore(NULL, search_data, best_k);

  // Append the nearest neighbors to the output vector in increasing distance correcting index permutations.
  while (!best_k.empty()) {
    Neighbor neighbor = best_k.back();
    neighbor.set_index(data_->get_original_index(neighbor.index()));
    output.push_back(neighbor);
    best_k.pop_back();
  }
}

/**
 * Find the K nearest neighbors of a given Point and push their indices sorted into a given STL vector.
 * In case that there are not enough points in the tree, all the available ones will be provided.
 *
 * \param p Point whose \a K neighbors should be retrieved.
 * \param distance Euclidean distance margin used to retrieve all points within.
 * \param output STL vector where the neighbors within the specified range will be appended. Elements are not sorted by distance.
 * \param metric Metric functor that will be used to calculate the distances between points. Defaults to Euclidean metric if C++1x is enabled.
 * \param ignore_p_in_tree Assume that \a p is contained in the tree any number of times and ignore them all.
 */
template <typename T, unsigned int D, typename L> template <typename Metric>
void KDTree<T, D, L>::all_in_range(const Vector &p, ConstRef_Distance distance, std::vector<Neighbor> &output, const Metric &metric, bool ignore_p_in_tree) const {

  // Check if there is any data on the tree and K is valid.
  KCHE_TREE_DCHECK(data_);
  if (!root_ || size() == 0 || !(distance > Traits<Distance>::zero()))
    return;

  // Create an object for tree traversal and incremental hyperrectangle intersection calculation.
  KDSearch<Element, Dimensions, Metric> search_data(p, *data_, metric, 0, ignore_p_in_tree);
  search_data.farthest_distance = distance;
  search_data.farthest_distance *= distance;

  // Build a STL vector to hold all the points in range.
  std::vector<Neighbor> points_in_range;

  // Store a dummy element in the vector with the distance range (will act as the farthest nearest neighbor during calculations).
  points_in_range.push_back(Neighbor(-1, search_data.farthest_distance));

  // Start an exploration traversal from the root.
  root_->explore(NULL, search_data, points_in_range);

  // Append the nearest neighbors to the output vector correcting index permutations.
  for (unsigned int i=1; i<points_in_range.size(); ++i) {
    Neighbor neighbor = points_in_range[i];
    neighbor.set_index(data_->get_original_index(neighbor.index()));
    output.push_back(neighbor);
  }
}

} // namespace kche_tree
