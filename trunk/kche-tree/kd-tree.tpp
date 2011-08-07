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
 * \file kd-tree.tpp
 * \brief Template implementations for generic kd-trees.
 * \author Leandro Graciá Gil
 */

namespace kche_tree {

// Include the out_of_range exception.
#include <stdexcept>

/**
 * Subscripting operator to access kd-tree data.
 *
 * \param index Index of the data being accessed.
 * \return Reference to the index-th element provided when building the tree (internal copy).
 */
template <typename T, const unsigned int D>
const typename KDTree<T, D>::VectorType &KDTree<T, D>::operator [] (unsigned int index) const {

  // Check boundaries.
  if (index >= size()) {
    std::string error_msg = "index out of range (requesting ";
    error_msg += index;
    error_msg += ", size ";
    error_msg += size();
    error_msg += ")";
    throw std::out_of_range(error_msg);
  }

  return data[inverse_permutation[index]];
}

/**
 * Build a kd-tree from a set of \a n D-dimensional samples.
 *
 * \param train_set Train set used to build the kd-tree.
 * \param bucket_size Number of elements that should be grouped in leaf nodes.
 * \return \c true if successful, \c false otherwise.
 */
template <typename T, const unsigned int D>
bool KDTree<T, D>::build(const DataSetType &train_set, unsigned int bucket_size) {

  // Check params.
  unsigned int num_points = train_set.size();
  if (num_points == 0 || bucket_size == 0)
    return false;

  // Reallocate permutation arrays if the new size is different from the existing one.
  if (data.size() != num_points) {
    permutation.reset(new unsigned int[num_points]);
    inverse_permutation.reset(new unsigned int[num_points]);
  }

  // Initialize the permutation array to identity.
  for (unsigned int i=0; i<num_points; ++i)
    permutation[i] = i;

  // Build the kd-tree recursively (num_elements will contain a recursively-calculated num_points after the call).
  unsigned int num_elements = 0;
  root.reset(KDNode<T, D>::build(train_set, permutation.get(), train_set.size(), NULL, bucket_size, num_elements));
  assert(num_elements == num_points);

  // Make a permutated copy of the input data and calculate the inverse permutation.
  data.reset_to_size(num_points);
  for (unsigned int i=0; i<num_points; ++i) {
    data[i] = train_set[permutation[i]];
    inverse_permutation[permutation[i]] = i;
  }

  return true;
}

/**
 * Find the K nearest neighbours of a given Point and push their indices sorted into a given STL vector.
 * In case that there are not enough points in the tree, all the available ones will be provided.
 *
 * \param p Point whose \a K neighbours should be retrieved.
 * \param K Number of nearest neighbours to retrieve.
 * \param output STL vector where the nearest neighbours will be appended. Sorted result depends on the template parameter S, enabled by default.
 * \param metric Metric functor that will be used to calculate the distances between points. Defaults to Euclidean metric if C++0x is enabled.
 * \param epsilon Acceptable distance margin to ignore regions during kd-tree exploration. Defaults to zero (deterministic).
 * \param ignore_p_in_tree Assume that \a p is contained in the tree any number of times and ignore them all.
 */
template <typename T, const unsigned int D> template <template <typename, typename> class KContainer, typename M>
void KDTree<T, D>::knn(const VectorType &p, unsigned int K, std::vector<NeighbourType> &output, const M &metric, ConstRef_T epsilon, bool ignore_p_in_tree) const {

  // Check if there is any data on the tree and K is valid.
  if (root == NULL || size() == 0 || K == 0)
    return;

  // Create an object for tree traversal and incremental hyperrectangle intersection calculation.
  KDSearchData<T, D, M> search_data(p, data, metric, K, ignore_p_in_tree);

  // Convert epsilon to a squared distance and set it as initial hyperrectangle distance.
  search_data.hyperrect_distance = epsilon;
  search_data.hyperrect_distance *= epsilon;

  // Build a special sorted container for the current K nearest neighbour candidates.
  KContainer<VectorDistance<T>, VectorDistance<T> > best_k(K);

  // Start an exploration traversal from the root.
  root->explore(NULL, search_data, best_k);

  // Append the nearest neighbours to the output vector in increasing distance correcting index permutations.
  while (!best_k.empty()) {
    NeighbourType neighbour = best_k.back();
    neighbour.index = permutation[neighbour.index];
    output.push_back(neighbour);
    best_k.pop_back();
  }
}

/**
 * Find the K nearest neighbours of a given Point and push their indices sorted into a given STL vector.
 * In case that there are not enough points in the tree, all the available ones will be provided.
 *
 * \param p Point whose \a K neighbours should be retrieved.
 * \param distance Euclidean distance margin used to retrieve all points within.
 * \param output STL vector where the neighbours within the specified range will be appended. Elements are not sorted by distance.
 * \param metric Metric functor that will be used to calculate the distances between points. Defaults to Euclidean metric if C++0x is enabled.
 * \param ignore_p_in_tree Assume that \a p is contained in the tree any number of times and ignore them all.
 */
template <typename T, const unsigned int D> template <typename M>
void KDTree<T, D>::all_in_range(const VectorType &p, ConstRef_T distance, std::vector<NeighbourType> &output, const M &metric, bool ignore_p_in_tree) const {

  // Check if there is any data on the tree and K is valid.
  if (root == NULL || size() == 0 || !(distance > Traits<T>::zero()))
    return;

  // Create an object for tree traversal and incremental hyperrectangle intersection calculation.
  KDSearchData<T, D, M> search_data(p, data, metric, 0, ignore_p_in_tree);
  search_data.farthest_distance = distance;
  search_data.farthest_distance *= distance;

  // Build a STL vector to hold all the points in range.
  std::vector<NeighbourType> points_in_range;

  // Store a dummy element in the vector with the distance range (will act as the farthest nearest neighbour during calculations).
  points_in_range.push_back(NeighbourType(-1, search_data.farthest_distance));

  // Start an exploration traversal from the root.
  root->explore(NULL, search_data, points_in_range);

  // Append the nearest neighbours to the output vector correcting index permutations.
  for (unsigned int i=1; i<points_in_range.size(); ++i) {
    NeighbourType neighbour = points_in_range[i];
    neighbour.index = permutation[neighbour.index];
    output.push_back(neighbour);
  }
}

} // namespace kche_tree
