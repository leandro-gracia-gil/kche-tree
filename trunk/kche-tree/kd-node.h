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

#include "vector.h"

namespace kche_tree {

/**
 * \brief Structure holding the data specific to search in the tree.
 *
 * Will be expanded with any extra data provided by the associated incremental calculation type.
 */
template <typename T, const unsigned int D, typename M>
struct KDSearchData : M::IncrementalType::SearchDataExtras {

  const Vector<T, D> &p; ///< Reference input point.
  const Vector<T, D> *data; ///< Pointer to permutated data array.
  unsigned int K; ///< Number of neighbours to retrieve.

  T hyperrect_distance; ///< Distance to the current nearest point in the hyperrectangle.
  T farthest_distance; ///< Current distance from the farthest nearest neighbour to the reference point.
  bool ignore_null_distances;  ///< Used to exclude the source point if it's already in the tree.

  M metric; ///< Metric functor used to calculate distances between points.

  /// Initialize data for a tree search with incremental intersection calculation.
  KDSearchData(const Vector<T, D> &p, const Vector<T, D> *data, unsigned int K, bool ignore_p_in_tree);
};

/// Kd-tree leaf node.
template <typename T, const unsigned int D>
struct KDLeaf {
  unsigned int first_index; ///< Index of the first element contained by the leaf node.
  unsigned int num_elements; ///< Number of elements contained by the node.

  /// Leaf constructor.
  KDLeaf(unsigned int first_index, unsigned int num_elements) :
    first_index(first_index), num_elements(num_elements) {}

  /// Construct from an input stream.
  KDLeaf(std::istream &input);

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
  bool write_to_binary_stream(std::ostream &out);

#ifdef KCHE_TREE_DEBUG
  template <typename Op>
  bool verify_op(const Vector<T, D> *data, float value, int axis, const Op &op) const;
#endif
};

/// Kd-tree branch node.
template <typename T, const unsigned int D>
struct KDNode {

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
    unsigned int axis; ///< Index of the current axis being split.
    unsigned int is_leaf; ///< Bitmask used to check if left and right nodes are leafs or branches.
  };

  // Bit masks to access the leaf and axis information.
  static const unsigned int left_bit  = 0x80000000U; ///< Mask used to access the left branch bit in is_leaf.
  static const unsigned int right_bit = 0x40000000U; ///< Mask used to access the right branch bit in is_leaf.
  static const unsigned int axis_mask = 0x3FFFFFFFU; ///< Mask used to access the axis bits.

  // Constructors and destructor.
  KDNode() : left_branch(NULL), right_branch(NULL), is_leaf(0) {} ///< Default constructor.
  KDNode(std::istream &input); ///< Construct from an input stream.
  ~KDNode(); ///< Default destructor.

#ifdef KCHE_TREE_DEBUG
  bool verify(const Vector<T, D> *data, int axis) const;

  template <typename Op>
  bool verify_op(const Vector<T, D> *data, float value, int axis, const Op &op) const;
#endif

  /// Per axis element comparison functor. Used to apply STL sorting algorithms to individual axes.
  struct AxisComparer {
    const Vector<T, D> *data; ///< Array of input data.
    unsigned int axis; ///< Current axis used for sorting.

    /// Axis-th element comparison. Used to perform per-dimension data sorting.
    bool operator () (const unsigned int &i1, const unsigned int &i2) const {
      return data[i1][axis] < data[i2][axis];
    }
  };

  // --- Training-related --- //

  /// Build the kd-tree recursively.
  static KDNode *build(const Vector<T, D> *data, unsigned int *index, unsigned int n,
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
  bool write_to_binary_stream(std::ostream &out);
};

} // namespace kche_tree

// Template implementation.
#include "kd-node.tpp"

#endif
