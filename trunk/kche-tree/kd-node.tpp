/***************************************************************************
 *   Copyright (C) 2011 by Leandro Graciá Gil                              *
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
 * \file kd-node.tpp
 * \brief Template implementations for kd-tree nodes and auxiliary structures.
 * \author Leandro Graciá Gil
 */

// Include STL sort and comparison functors.
#include <algorithm>
#include <functional>

namespace kche_tree {

/**
 * Build a kd-tree recursively.
 *
 * \param data Base of the data array.
 * \param indices Array of indices to D-dimensional data vectors.
 * \param n Number of elements in \a index.
 * \param parent Parent node.
 * \param bucket_size Number of elements that should be grouped in leaf nodes.
 * \param processed Number of elements already processed and stored in the tree. Updated as the building expands.
 * \return Node of the tree completely initialized.
 */
template <typename T, const unsigned int D>
KDNode<T, D> *KDNode<T, D>::build(const DataSetType &data, unsigned int *indices, unsigned int n,
    KDNode *parent, unsigned int bucket_size, unsigned int &processed) {

  // Handle empty nodes (only for degenerate bucket sizes).
  if (n == 0)
    return NULL;

  // Allocate a new node.
  KDNode *node = new KDNode();

  // Split the data with a basic cycle over the dimension indices.
  node->axis = parent ? (parent->axis + 1) % D : 0;

  // Create a sorter for the current axis.
  AxisComparer comparer = { data, node->axis };

  // Find a pivot to split data appropiately (may involve index sorting or partitioning).
  unsigned int pivot = node->split(indices, n, comparer);

  // Split the data in two segments: left to pivot inclusive, and elements right to it.
  unsigned int left_elements = pivot + 1;
  unsigned int right_elements = n - left_elements;
  unsigned int *right_indices = indices + left_elements;

  // Store the axis-th value of the pivot used to split the hyperspace in two.
  node->split_value = data[indices[pivot]][node->axis];

  // Process the left part recursively, creating a leaf is remaining data is not greater than the bucket size.
  if (left_elements > bucket_size)
    node->left_branch = build(data, indices, left_elements, node, bucket_size, processed);
  else {
    node->left_leaf = new KDLeaf<T, D>(processed, left_elements);
    node->is_leaf |= left_bit;
    processed += left_elements;
  }

  // Process the right part recursively, creating a leaf is remaining data is not greater than the bucket size.
  if (right_elements > bucket_size)
    node->right_branch = build(data, right_indices, right_elements, node, bucket_size, processed);
  else {
    node->right_leaf = new KDLeaf<T, D>(processed, right_elements);
    node->is_leaf |= right_bit;
    processed += right_elements;
  }

  // Return processed node.
  return node;
}

/**
 * Split the provided data subset by one dimension. Should be near to the median to get a balanced kd-tree.
 * The dimension used to split the data is also decided by this method.
 * Any index sorting or partitioning must be also performed here.
 *
 * \param indices Array of indices to elements of the current data subset.
 * \param n Number of elements in \a index.
 * \param comparer Functor object used to compare data elements. Can be used with STL comparison-based algorithms.
 *
 * \return The index of the pivot element in the index array used to split the space.
 * All data in the left half must be less than the value associated to this index.
 */
template <typename T, const unsigned int D>
unsigned int KDNode<T, D>::split(unsigned int *indices, unsigned int n, const AxisComparer &comparer) {

  // Avoid sorting when less than 2 elements (base case).
  if (n < 2)
    return 0;

  // Sort the indices (a partial sort was used previously, but it was slower due to the internal use of STL heaps).
  std::sort(indices, indices + n, comparer);

  // Return the index of the median.
  unsigned int median = ((n + 1) >> 1) - 1;
  return median;
}

/**
 * Default node destructor.
 * Delete the tree recursively handling branches and leaf nodes appropiately.
 */
template <typename T, const unsigned int D>
KDNode<T, D>::~KDNode() {

  // Delete left branch / leaf.
  if (is_leaf & left_bit)
    delete left_leaf;
  else
    delete left_branch;

  // Delete right branch / leaf.
  if (is_leaf & right_bit)
    delete right_leaf;
  else
    delete right_branch;
}

/**
 * Initialize a data searching structure with incremental hyperrectangle intersection calculation.
 *
 * \param p Reference point being used in the search.
 * \param data Permutated training set stored by the tree.
 * \param metric Metric functor used for the distance calculations.
 * \param K Number of neighbours to retrieve.
 * \param ignore_null_distances_arg Indicate that points with null distance should be ignored.
 */
template <typename T, const unsigned int D, typename M>
KDSearchData<T, D, M>::KDSearchData(const VectorType &p, const DataSetType &data, const M &metric, unsigned int K, bool ignore_null_distances_arg)
  : M::IncrementalUpdaterType::SearchDataExtras(p, data),
    p(p),
    data(data),
    metric(metric),
    K(K),
    hyperrect_distance(Traits<T>::zero()),
    farthest_distance(Traits<T>::zero()),
    ignore_null_distances(ignore_null_distances_arg) {}

/**
 * Traverse the kd-tree looking for nearest neighbours candidates, but do not discard any regions of the space.
 *
 * \param parent Parent node of the one being explored.
 * \param search_data Auxiliar data structure used for tree traversal and incremental calculations.
 * \param candidates STL container-like object holding the current neighbour candidates.
 */
template <typename T, const unsigned int D> template <typename M, typename C>
void KDNode<T, D>::explore(const KDNode *parent, KDSearchData<T, D, M> &search_data, C &candidates) const {

  // Intersection data is updated incrementally when the object is created, and restored when destroyed.
  typename M::IncrementalUpdaterType incremental_update(this, parent, search_data);

  // Check which branch should be explored first.
  KDNode<T, D> *first_branch = NULL, *second_branch = NULL;
  KDLeaf<T, D> *first_leaf = NULL, *second_leaf = NULL;

  // Left branch first or same point.
  if (!(search_data.p[axis & axis_mask] > split_value)) {
    if (is_leaf & left_bit)
      first_leaf = left_leaf;
    else
      first_branch = left_branch;

    if (is_leaf & right_bit)
      second_leaf = right_leaf;
    else
      second_branch = right_branch;
  }
  // Right branch first.
  else {
    if (is_leaf & right_bit)
      first_leaf = right_leaf;
    else
      first_branch = right_branch;

    if (is_leaf & left_bit)
      second_leaf = left_leaf;
    else
      second_branch = left_branch;
  }

  // Traverse the first (manhattan nearest) branch.
  bool full = candidates.size() >= search_data.K;
  if (first_leaf != NULL) {
    if (full)
      first_leaf->intersect(search_data, candidates);
    else
      first_leaf->explore(search_data, candidates);
  }
  else {
    if (full)
      first_branch->intersect(this, search_data, candidates);
    else
      first_branch->explore(this, search_data, candidates);
  }

  // Traverse the second (manhattan farthest) branch.
  full = candidates.size() >= search_data.K;
  if (second_leaf != NULL) {
    if (full)
      second_leaf->intersect(search_data, candidates);
    else
      second_leaf->explore(search_data, candidates);
  }
  else {
    if (full)
      second_branch->intersect(this, search_data, candidates);
    else
      second_branch->explore(this, search_data, candidates);
  }
}

/**
 * Traverse the kd-tree while discarding regions of space with hyperrectangle intersections.
 *
 * \param parent Parent node of the one being explored.
 * \param search_data Auxiliar data structure used for tree traversal and incremental calculations.
 * \param candidates STL container-like object holding the current neighbour candidates.
 */
template <typename T, const unsigned int D> template <typename M, typename C>
void KDNode<T, D>::intersect(const KDNode<T, D> *parent, KDSearchData<T, D, M> &search_data, C &candidates) const {

  // Intersection data is updated incrementally when the object is created, and restored when destroyed.
  typename M::IncrementalUpdaterType incremental_update(this, parent, search_data);

  // Check if the volume defined by the distance from current worst neighbour candidate intersects the region hyperrectangle.
  if (!(search_data.hyperrect_distance < search_data.farthest_distance))
    return;

  // Traverse left branch discarding regions of space.
  if (is_leaf & left_bit) {
    if (search_data.ignore_null_distances)
      left_leaf->intersect_ignoring_same(search_data, candidates);
    else
      left_leaf->intersect(search_data, candidates);
  } else {
    left_branch->intersect(this, search_data, candidates);
  }

  // Traverse right branch discarding regions of space.
  if (is_leaf & right_bit) {
    if (search_data.ignore_null_distances)
      right_leaf->intersect_ignoring_same(search_data, candidates);
    else
      right_leaf->intersect(search_data, candidates);
  } else {
    right_branch->intersect(this, search_data, candidates);
  }
}

/**
 * Process a leaf node that contains many buckets. Do not use any upper bounds in distance calculation (otherwise best candidates may be skipped).
 *
 * \param search_data Auxiliar data structure used for tree traversal and incremental calculations.
 * \param candidates STL container-like object holding the current neighbour candidates.
 */
template <typename T, const unsigned int D> template <typename M, typename C>
void KDLeaf<T, D>::explore(KDSearchData<T, D, M> &search_data, C &candidates) const {

  if (search_data.ignore_null_distances) {

    // Process only the bucket elements different to p.
    for (unsigned int i=first_index; i < first_index + num_elements; ++i) {
      ConstRef_T distance = search_data.metric(search_data.p, search_data.data[i]);
      if (distance > Traits<T>::zero())
        candidates.push_back(VectorDistance<T>(i, search_data.metric(search_data.p, search_data.data[i])));
    }

  } else {
    // Process all the buckets in the node.
    for (unsigned int i=first_index; i < first_index + num_elements; ++i)
      // Create a new neighbour candidate with the point referenced by this node and push it into the K best ones.
      candidates.push_back(VectorDistance<T>(i, search_data.metric(search_data.p, search_data.data[i])));
  }

  // Update current farthest nearest neighbour distance.
  if (!candidates.empty())
    search_data.farthest_distance = candidates.front().squared_distance;
}

/**
 * Process a leaf node that contains many buckets without trying to discard them.
 *
 * \param search_data Auxiliar data structure used for tree traversal and incremental calculations.
 * \param candidates STL container-like object holding the current neighbour candidates.
 */
template <typename T, const unsigned int D> template <typename M, typename C>
void KDLeaf<T, D>::intersect(KDSearchData<T, D, M> &search_data, C &candidates) const {

  // Process all the buckets in the node.
  for (unsigned int i=first_index; i < first_index + num_elements; ++i) {

    // Calculate the distance to the new candidate, upper bounded by the farthest nearest neighbour distance.
    ConstRef_T new_distance = search_data.metric(search_data.p, search_data.data[i], search_data.farthest_distance);

    // If less or equal than the current farthest nearest neighbour then it's a valid candidate (equal is left for the all_in_range method).
    if (!(new_distance > search_data.farthest_distance)) {

      // Push it in the nearest neighbour container (will reject the previous farthest one).
      candidates.push_back(VectorDistance<T>(i, new_distance));

      // Update the distance to the new farthest nearest neighbour.
      search_data.farthest_distance = candidates.front().squared_distance;
    }
  }
}

/**
 * Process a leaf node that contains many buckets without trying to discard them.
 * Ignore any points with distance 0.
 *
 * \param search_data Auxiliar data structure used for tree traversal and incremental calculations.
 * \param candidates STL container-like object holding the current neighbour candidates.
 */
template <typename T, const unsigned int D> template <typename M, typename C>
void KDLeaf<T, D>::intersect_ignoring_same(KDSearchData<T, D, M> &search_data, C &candidates) const {

  // Process all the buckets in the node.
  for (unsigned int i=first_index; i < first_index + num_elements; ++i) {

    // Calculate the distance to the new candidate, upper bounded by the farthest nearest neighbour distance.
    ConstRef_T new_distance = search_data.metric(search_data.p, search_data.data[i], search_data.farthest_distance);
    if (new_distance == Traits<T>::zero())
      continue;

    // If less or equal than the current farthest nearest neighbour then it's a valid candidate (equal is left for the all_in_range method).
    if (!(new_distance > search_data.farthest_distance)) {

      // Push it in the nearest neighbour container (will reject the previous farthest one).
      candidates.push_back(VectorDistance<T>(i, new_distance));

      // Update the distance to the new farthest nearest neighbour.
      search_data.farthest_distance = candidates.front().squared_distance;
    }
  }
}

template <typename T, const unsigned int D>
void KDNode<T, D>::verify_properties(const DataSetType &data, int axis) const {

  // Alias to STL comparison functors.
  typedef std::less<T> LessFunc; // Negated to get greater_equal without requiring additional operators.
  typedef std::greater<T> GreaterFunc; // Negated to get less_equal without requiring additional operators.

  if (is_leaf & left_bit)
    left_leaf->verify_properties(data, split_value, axis, std::binary_negate<GreaterFunc>(GreaterFunc()));
  else
    left_branch->verify_properties(data, split_value, axis, std::binary_negate<GreaterFunc>(GreaterFunc()));

  if (is_leaf & right_bit)
    right_leaf->verify_properties(data, split_value, axis, std::binary_negate<LessFunc>(LessFunc()));
  else
    right_branch->verify_properties(data, split_value, axis, std::binary_negate<LessFunc>(LessFunc()));

  if (!(is_leaf & left_bit))
    left_branch->verify_properties(data, (axis + 1) % D);

  if (!(is_leaf & right_bit))
    right_branch->verify_properties(data, (axis + 1) % D);
}

template <typename T, const unsigned int D> template <typename Op>
void KDNode<T, D>::verify_properties(const DataSetType &data, ConstRef_T value, int axis, const Op &op) const {

  if (is_leaf & left_bit)
    left_leaf->verify_properties(data, value, axis, op);
  else
    left_branch->verify_properties(data, value, axis, op);

  if (is_leaf & right_bit)
    right_leaf->verify_properties(data, value, axis, op);
  else
    right_branch->verify_properties(data, value, axis, op);
}

template <typename T, const unsigned int D> template <typename Op>
void KDLeaf<T, D>::verify_properties(const DataSetType &data, ConstRef_T value, int axis, const Op &op) const {

  for (uint32_t i=first_index; i < first_index + num_elements; ++i) {
    if (!op(data[i][axis], value)) {
      std::string error_msg = "kd-tree structural error on axis ";
      error_msg += axis;
      throw std::runtime_error(error_msg);
    }
  }
}

} // namespace kche_tree
