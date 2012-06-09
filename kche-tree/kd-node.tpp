/***************************************************************************
 *   Copyright (C) 2011, 2012 by Leandro Graciá Gil                        *
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

#include "neighbor.h"

namespace kche_tree {

/**
 * \brief Build a kd-tree recursively.
 *
 * \param data Base of the data array.
 * \param indices Array of indices to D-dimensional data vectors.
 * \param n Number of elements in \a index.
 * \param parent Parent node.
 * \param bucket_size Number of elements that should be grouped in leaf nodes.
 * \param processed Number of elements already processed and stored in the tree. Updated as the building expands.
 * \return Node of the tree completely initialized.
 */
template <typename T, unsigned int D>
KDNode<T, D>* KDNode<T, D>::build(const DataSet &data, unsigned int *indices, unsigned int n,
    KDNode *parent, unsigned int bucket_size, unsigned int &processed) {

  // Handle empty nodes (only for degenerate bucket sizes).
  if (n == 0)
    return NULL;

  // Allocate a new node.
  KDNode *node = new KDNode();

  // Split the data with a basic cycle over the dimension indices.
  node->axis = parent ? (parent->axis + 1) % Dimensions : 0;

  // Create a sorter for the current axis.
  AxisComparer comparer = { data, node->axis };

  // Find a pivot to split data appropiately (may involve index sorting or partitioning).
  unsigned int pivot = node->split(indices, n, comparer);

  // Split the data in two segments: left to pivot inclusive, and elements right to it.
  unsigned int left_elements = pivot + 1;
  unsigned int right_elements = n - left_elements;
  unsigned int *right_indices = indices + left_elements;

  // Store the axis-th element of the pivot used to split the hyperspace in two.
  node->split_element = data[indices[pivot]][node->axis];

  // Process the left part recursively, creating a leaf is remaining data is not greater than the bucket size.
  if (left_elements > bucket_size)
    node->left_branch = build(data, indices, left_elements, node, bucket_size, processed);
  else {
    node->left_leaf = new KDLeaf(processed, left_elements);
    node->is_leaf |= left_bit;
    processed += left_elements;
  }

  // Process the right part recursively, creating a leaf is remaining data is not greater than the bucket size.
  if (right_elements > bucket_size)
    node->right_branch = build(data, right_indices, right_elements, node, bucket_size, processed);
  else {
    node->right_leaf = new KDLeaf(processed, right_elements);
    node->is_leaf |= right_bit;
    processed += right_elements;
  }

  // Return processed node.
  return node;
}

/**
 * \brief Split the provided data subset by one dimension. Should be near to the median to get a balanced kd-tree.
 *
 * The dimension used to split the data is also decided by this method.
 * Any index sorting or partitioning must be also performed here.
 *
 * \param indices Array of indices to elements of the current data subset.
 * \param n Number of elements in \a index.
 * \param comparer Functor object used to compare data elements. Can be used with STL comparison-based algorithms.
 *
 * \return The index of the pivot element in the index array used to split the space.
 * All data in the left half must be less or equal than the value associated to this index.
 */
template <typename T, unsigned int D>
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
 * \brief Default node destructor.
 *
 * Delete the tree recursively handling branches and leaf nodes appropiately.
 */
template <typename T, unsigned int D>
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
 * \brief Traverse the kd-tree looking for nearest neighbours candidates, but do not discard any regions of the space.
 *
 * \param parent Parent node of the one being explored.
 * \param search_data Auxiliar data structure used for tree traversal and incremental calculations.
 * \param candidates STL container-like object holding the current neighbour candidates.
 */
template <typename T, unsigned int D> template <typename Metric, typename Container>
void KDNode<T, D>::explore(const KDNode *parent, KDSearch<T, D, Metric> &search_data, Container &candidates) const {

  // Intersection data is updated incrementally when the object is created, and restored when destroyed.
  typename Metric::IncrementalUpdater incremental_update(this, parent, search_data);

  // Check which branch should be explored first.
  KDNode *first_branch = NULL, *second_branch = NULL;
  KDLeaf *first_leaf = NULL, *second_leaf = NULL;

  // Left branch first or same point.
  if (!(search_data.p[axis & axis_mask] > split_element)) {
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
 * \brief Traverse the kd-tree while discarding regions of space with hyperrectangle intersections.
 *
 * \param parent Parent node of the one being explored.
 * \param search_data Auxiliar data structure used for tree traversal and incremental calculations.
 * \param candidates STL container-like object holding the current neighbour candidates.
 */
template <typename T, unsigned int D> template <typename Metric, typename Container>
void KDNode<T, D>::intersect(const KDNode *parent, KDSearch<T, D, Metric> &search_data, Container &candidates) const {

  // Intersection data is updated incrementally when the object is created, and restored when destroyed.
  typename Metric::IncrementalUpdater incremental_update(this, parent, search_data);

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
 * \brief Process a leaf node without using any upper bounds in distance calculation.
 *
 * \param search_data Auxiliar data structure used for tree traversal and incremental calculations.
 * \param candidates STL container-like object holding the current neighbour candidates.
 */
template <typename T, unsigned int D> template <typename Metric, typename Container>
void KDLeaf<T, D>::explore(KDSearch<T, D, Metric> &search_data, Container &candidates) const {

  if (search_data.ignore_null_distances) {
    // Process only the bucket elements different to p.
    for (unsigned int i=first_index; i < first_index + num_elements; ++i) {
      ConstRef_Distance distance = search_data.metric(search_data.p, search_data.data.get_permuted(i));
      if (distance > Traits<Distance>::zero())
        candidates.push_back(Neighbor<Distance>(i, search_data.metric(search_data.p, search_data.data.get_permuted(i))));
    }

  } else {
    // Process all the buckets in the node.
    for (unsigned int i=first_index; i < first_index + num_elements; ++i)
      // Create a new neighbour candidate with the point referenced by this node and push it into the K best ones.
      candidates.push_back(Neighbor<Distance>(i, search_data.metric(search_data.p, search_data.data.get_permuted(i))));
  }

  // Update current farthest nearest neighbour distance.
  if (!candidates.empty())
    search_data.farthest_distance = candidates.front().squared_distance();
}

/**
 * \brief Process a leaf node using an upper bound in the corresponding metric.
 *
 * \param search_data Auxiliar data structure used for tree traversal and incremental calculations.
 * \param candidates STL container-like object holding the current neighbour candidates.
 */
template <typename T, unsigned int D> template <typename Metric, typename Container>
void KDLeaf<T, D>::intersect(KDSearch<T, D, Metric> &search_data, Container &candidates) const {

  // Process all the buckets in the node.
  for (unsigned int i=first_index; i < first_index + num_elements; ++i) {

    // Calculate the distance to the new candidate, upper bounded by the farthest nearest neighbour distance.
    ConstRef_Distance new_distance = search_data.metric(search_data.p, search_data.data.get_permuted(i), search_data.farthest_distance);

    // If less or equal than the current farthest nearest neighbour then it's a valid candidate (equal is left for the all_in_range method).
    if (!(new_distance > search_data.farthest_distance)) {

      // Push it in the nearest neighbour container (will reject the previous farthest one).
      candidates.push_back(Neighbor<Distance>(i, new_distance));

      // Update the distance to the new farthest nearest neighbour.
      search_data.farthest_distance = candidates.front().squared_distance();
    }
  }
}

/**
 * \brief Process a leaf node using an upper bound in the corresponding metric. Ignores any points with distance 0.
 *
 * \param search_data Auxiliar data structure used for tree traversal and incremental calculations.
 * \param candidates STL container-like object holding the current neighbour candidates.
 */
template <typename T, unsigned int D> template <typename Metric, typename Container>
void KDLeaf<T, D>::intersect_ignoring_same(KDSearch<T, D, Metric> &search_data, Container &candidates) const {

  // Process all the buckets in the node.
  for (unsigned int i=first_index; i < first_index + num_elements; ++i) {

    // Calculate the distance to the new candidate, upper bounded by the farthest nearest neighbour distance.
    ConstRef_Distance new_distance = search_data.metric(search_data.p, search_data.data.get_permuted(i), search_data.farthest_distance);
    if (new_distance == Traits<Distance>::zero())
      continue;

    // If less or equal than the current farthest nearest neighbour then it's a valid candidate (equal is left for the all_in_range method).
    if (!(new_distance > search_data.farthest_distance)) {

      // Push it in the nearest neighbour container (will reject the previous farthest one).
      candidates.push_back(Neighbor<Distance>(i, new_distance));

      // Update the distance to the new farthest nearest neighbour.
      search_data.farthest_distance = candidates.front().squared_distance();
    }
  }
}

/**
 * \brief Verifies the structural integrity of the kd-tree branch hanging by this node.
 *
 * This method will check that the expected structural properties of the kd-tree hold.
 * Specifically, it will ensure that elements left and right of the split value are respectively <= or >= than it
 * in every dimension along the tree branch.
 *
 * \param data Data set the branch refers to.
 * \param axis Index of the dimension where the kd-tree properties should be verified.
 * \exception std::runtime_error if the structure is invalid.
 */
template <typename T, unsigned int D>
void KDNode<T, D>::verify_properties(const DataSet &data, unsigned int axis) const {

  // Alias to STL comparison functors.
  typedef std::less<Element> LessFunc; // Negated to get greater_equal without requiring additional operators.
  typedef std::greater<Element> GreaterFunc; // Negated to get less_equal without requiring additional operators.

  bool is_left_leaf = is_leaf & left_bit;
  bool is_right_leaf = is_leaf & right_bit;

  // Verify the structural properties along this dimension in the rest of the tree.
  if (is_left_leaf)
    left_leaf->verify_properties(data, axis, split_element, std::binary_negate<GreaterFunc>(GreaterFunc()));
  else
    left_branch->verify_properties(data, axis, split_element, std::binary_negate<GreaterFunc>(GreaterFunc()));

  if (is_right_leaf)
    right_leaf->verify_properties(data, axis, split_element, std::binary_negate<LessFunc>(LessFunc()));
  else
    right_branch->verify_properties(data, axis, split_element, std::binary_negate<LessFunc>(LessFunc()));

  // Recursively verify the next dimensions.
  if (!is_left_leaf)
    left_branch->verify_properties(data, (axis + 1) % Dimensions);

  if (!is_right_leaf)
    right_branch->verify_properties(data, (axis + 1) % Dimensions);
}

/**
 * \brief Verifies the structural integrity of a kd-tree branch in the provided dimension.
 *
 * Checks that all elements in a dimension verify a provided comparison related to its split element.
 *
 * \param data Data set the branch refers to.
 * \param axis Index of the dimension where the kd-tree properties should be verified.
 * \param split_element Value splitting the \a axis dimension in two.
 * \param op Comparison operation to apply between the stored elements and the split value.
 * \exception std::runtime_error if the structure is invalid.
 */
template <typename T, unsigned int D> template <typename Op>
void KDNode<T, D>::verify_properties(const DataSet &data, unsigned int axis, ConstRef_Element split_element, const Op &op) const {

  // Recursively perform the verification over all the kd-tree.
  if (is_leaf & left_bit)
    left_leaf->verify_properties(data, axis, split_element, op);
  else
    left_branch->verify_properties(data, axis, split_element, op);

  if (is_leaf & right_bit)
    right_leaf->verify_properties(data, axis, split_element, op);
  else
    right_branch->verify_properties(data, axis, split_element, op);
}

/**
 * \brief Verifies the structural integrity of a kd-tree leaf node.
 *
 * \param data Data set the branch refers to.
 * \param axis Index of the dimension where the kd-tree properties should be verified.
 * \param split_element Value splitting the \a axis dimension in two.
 * \param op Comparison operation to apply between the stored elements and the split value.
 * \exception std::runtime_error if the structure is invalid.
 */
template <typename T, unsigned int D> template <typename Op>
void KDLeaf<T, D>::verify_properties(const DataSet &data, unsigned int axis, ConstRef_Element split_element, const Op &op) const {

  // Verify the bucket contained by the leaf node.
  for (uint32_t i=first_index; i < first_index + num_elements; ++i) {
    if (!op(data.get_permuted(i)[axis], split_element)) {
      std::string error_msg = "kd-tree structural error on axis ";
      error_msg += axis;
      throw std::runtime_error(error_msg);
    }
  }
}

} // namespace kche_tree
