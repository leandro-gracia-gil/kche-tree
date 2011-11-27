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
 * \file kd-node.h
 * \brief Templates for kd-tree nodes and auxiliary structures.
 * \author Leandro Graciá Gil
*/

#ifndef _KCHE_TREE_KD_NODE_H_
#define _KCHE_TREE_KD_NODE_H_

// Include type traits, feature vectors and optimized params.
#include "traits.h"
#include "vector.h"
#include "utils.h"

namespace kche_tree {

/**
 * \brief Structure holding the data specific to search in the tree.
 *
 * Will be expanded with any extra data provided by the associated incremental calculation type.
 */
template <typename T, const unsigned int D, typename M>
struct KDSearchData : M::IncrementalUpdaterType::SearchDataExtras {

  /// Use the global data set type by default.
  typedef typename TypeSettings<T, D>::DataSetType DataSetType;

  /// Use the global vector type by default.
  typedef typename TypeSettings<T, D>::VectorType VectorType;

  const VectorType &p; ///< Reference input point.
  const DataSetType &data; ///< Permutated training set.
  M metric; ///< Metric functor used to calculate distances between points.
  unsigned int K; ///< Number of neighbours to retrieve.

  T hyperrect_distance; ///< Distance to the current nearest point in the hyperrectangle.
  T farthest_distance; ///< Current distance from the farthest nearest neighbour to the reference point.
  bool ignore_null_distances;  ///< Used to exclude the source point if it's already in the tree.

  /// Initialize data for a tree search with incremental intersection calculation.
  KDSearchData(const VectorType &p, const DataSetType &data, const M &metric, unsigned int K, bool ignore_p_in_tree);
};

/// Kd-tree leaf node.
template <typename T, const unsigned int D>
struct KDLeaf {
  /// Use the global data set type by default.
  typedef typename TypeSettings<T, D>::DataSetType DataSetType;

  /// Use optimized const reference types.
  typedef typename RParam<T>::Type ConstRef_T;

  uint32_t first_index; ///< Index of the first element contained by the leaf node.
  uint32_t num_elements; ///< Number of elements contained by the node.

  /// Leaf constructor.
  KDLeaf(uint32_t first_index, uint32_t num_elements) :
    first_index(first_index), num_elements(num_elements) {}

  /// Construct from an input stream.
  KDLeaf(std::istream &input, Endianness::Type endianness = Endianness::endianness());

  /// Process a leaf node with many buckets on it. Do not use any upper bounds on distance.
  template <typename M, typename C>
  void explore(KDSearchData<T, D, M> &data, C &candidates) const;

  /// Process a leaf node with many buckets on it. Allows partial distance calculations.
  template <typename M, typename C>
  void intersect(KDSearchData<T, D, M> &data, C &candidates) const;

  /// Process a leaf node ignoring any existing instances of the reference point defined in \a data.
  template <typename M, typename C>
  void intersect_ignoring_same(KDSearchData<T, D, M> &data, C &candidates) const;

  /// Write to stream.
  void write_to_binary_stream(std::ostream &out);

  /// Verify the kd-tree properties using the provided functor. Throws std::runtime_error if invalid.
  template <typename Op>
  void verify_properties(const DataSetType &data, ConstRef_T value, int axis, const Op &op) const;
};

/// Kd-tree branch node.
template <typename T, const unsigned int D>
struct KDNode {

  /// Use the global data set type by default.
  typedef typename TypeSettings<T, D>::DataSetType DataSetType;

  /// Use optimized const reference types.
  typedef typename RParam<T>::Type ConstRef_T;

  union {
    KDNode<T, D> *left_branch; ///< Left branch.
    KDLeaf<T, D> *left_leaf; ///< Left leaf.
  };

  union {
    KDNode<T, D> *right_branch; ///< Right branch.
    KDLeaf<T, D> *right_leaf; ///< Right leaf.
  };

  T split_value; ///< Value used to split the hyperspace in two.

  // The leaf flags use the two uppermost bits of the axis index. Won't handle more than 2^30 dimensions.
  union {
    uint32_t axis; ///< Index of the current axis being split.
    uint32_t is_leaf; ///< Bitmask used to check if left and right nodes are leafs or branches.
  };

  // Bit masks to access the leaf and axis information.
  static const uint32_t left_bit  = 0x80000000U; ///< Mask used to access the left branch bit in is_leaf.
  static const uint32_t right_bit = 0x40000000U; ///< Mask used to access the right branch bit in is_leaf.
  static const uint32_t axis_mask = 0x3FFFFFFFU; ///< Mask used to access the axis bits.

  // Constructors and destructor.
  KDNode() : left_branch(NULL), right_branch(NULL), is_leaf(0) {} ///< Default constructor.
  KDNode(std::istream &input, Endianness::Type endianness = Endianness::endianness()); ///< Construct from an input stream.
  ~KDNode(); ///< Default destructor.

  /// Verify the kd-tree properties from the local node. Throws std::runtime_error if invalid.
  void verify_properties(const DataSetType &data, int axis) const;

  /// Verify the kd-tree properties using the provided functor. Throws std::runtime_error if invalid.
  template <typename Op>
  void verify_properties(const DataSetType &data, ConstRef_T value, int axis, const Op &op) const;

  /// Per axis element comparison functor. Used to apply STL sorting algorithms to individual axes.
  struct AxisComparer {
    const DataSetType &data; ///< Input training set.
    unsigned int axis; ///< Current axis used for sorting.

    /// Axis-th element comparison. Used to perform per-dimension data sorting.
    bool operator () (const unsigned int &i1, const unsigned int &i2) const {
      return data[i1][axis] < data[i2][axis];
    }
  };

  // --- Training-related --- //

  /// Build the kd-tree recursively.
  static KDNode *build(const DataSetType &data, unsigned int *index, unsigned int n,
      KDNode *parent, unsigned int bucket_size, unsigned int &processed);

  /// Find a pivot to split the space in two by a chosen dimension during training.
  unsigned int split(unsigned int *index, unsigned int n, const AxisComparer &comparer);

  // --- Search-related --- //

  /// Traverse the kd-tree looking for nearest neighbours candidates based on Manhattan distances.
  template <typename M, typename C>
  void explore(const KDNode<T, D> *parent, KDSearchData<T, D, M> &data, C &candidates) const;

  /// Traverse the kd-tree checking hyperrectangle intersections to discard regions of the space.
  template <typename M, typename C>
  void intersect(const KDNode<T, D> *parent, KDSearchData<T, D, M> &data, C &candidates) const;

  // --- IO-related --- //

  /// Write to stream.
  void write_to_binary_stream(std::ostream &out);
};

} // namespace kche_tree

// Template implementation.
#include "kd-node.tpp"

#endif
