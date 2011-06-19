/***************************************************************************
 *   Copyright (C) 2010 by Leandro Graciá Gil                              *
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
 * \file kd-tree.h
 * \brief Template for generic kd-trees.
 * \author Leandro Graciá Gil
*/

#ifndef _KCHE_TREE_KD_TREE_H_
#define _KCHE_TREE_KD_TREE_H_

// Include STL vectors for K neighbours output.
#include <vector>

// STL streams.
#include <iostream>

// Include k-neighbours containers (k-heaps and k-vectors).
#include "k-heap.h"
#include "k-vector.h"

// Include template for feature vectors and data sets.
#include "dataset.h"
#include "vector.h"

namespace kche_tree {

// Forward-declare the class.
template <typename T, const unsigned int D, typename S>
class KDTree;

// Forward-declare the stream operators.
template <typename T, const unsigned int D, typename S>
std::istream & operator >> (std::istream &in, KDTree<T, D, S> &kdtree); ///< Input operator for kd-trees (read from stream). Throws an exception in case of error.

template <typename T, const unsigned int D, typename S>
std::ostream & operator << (std::ostream &out, const KDTree<T, D, S> &kdtree); ///< Output operator for kd-trees (save to stream).

/**
 * \brief Template for generic cache-aware kd-trees of any type.
 *
 * Element insertions and deletions are not currently supported in behalf of a design based on cache efficiency.
 * \warning This class is currently not thread-safe. This feature will be added in future releases.
 *
 * \tparam T Data type of the elements in the kd-tree. Requires copy, internal + += = *, external (* int) and comparison < <= > operators.
 * \tparam D Number of dimensions in the input data.
 * \tparam S (Optional) K-best elements container. Responds to empty, size, back, push_back and pop_front.
 *  Defaults to KVector<VectorDistance<T>, VectorDistance<T> >, but KHeap<VectorDistance<T>, VectorDistance<T> > is also valid.
 */
template <typename T, const unsigned int D, typename S = KVector<VectorDistance<T>, VectorDistance<T> > >
class KDTree {
public:
  /// Define the number of dimensions of the kd-tree.
  static unsigned const int Dimensions = D;

  /// Consider compatible feature vectors as D-dimensional points in the space.
  typedef Vector<T, D> Point;

  /// Define the type used for kd-tree neighbours.
  typedef VectorDistance<T> Neighbour;

  // Constructors and destructors.
  KDTree(); ///< Default constructor. Creates an empty and uninitialized kd-tree.
  ~KDTree(); ///< Default destructor.

  // Basic kd-tree operations.
  bool build(const DataSet<T, D>& train_set, unsigned int bucket_size = 32); ///< Build a kd-tree from a set of training vectors. Cost: O(n log² n).
  void knn(const Point &p, unsigned int K, std::vector<Neighbour> &output, T epsilon = (T) 0, bool ignore_p_in_tree = false) const; ///< Get the K nearest neighbours of a point. Estimated average cost: O(log K log n).
  void all_in_range(const Point &p, T distance, std::vector<Neighbour> &output, bool ignore_p_in_tree = false) const; ///< Get all neighbours within a distance from a point. Estimated average Cost: O(log m log n) depending on the number of results m.

  // Subscript operator for accesing stored data (will fail on non-built kd-trees).
  const Point &operator [] (unsigned int index) const;

  // Stream operators.
  friend std::istream &operator >> <>(std::istream &in, KDTree &kdtree);
  friend std::ostream &operator << <>(std::ostream &out, const KDTree &kdtree);

  // Kd-tree properties.
  unsigned int size() const { return num_elements; } ///< Get the number of elements stored in the tree.

#ifdef DEBUG_KDTREE
  bool verify() const;
#endif

protected:
  /// Structure holding data for incremental hyperrectangle-hypersphere intersection and nearest neighbour search.
  struct SearchData {

    const Point &p; ///< Reference input point.
    const Point *data; ///< Pointer to permutated data array.
    unsigned int K; ///< Number of neighbours to retrieve.

    T hyperrect_distance; ///< Distance to the current nearest point in the hyperrectangle.
    T farthest_distance; ///< Current distance from the farthest nearest neighbour to the reference point.
    bool ignore_null_distances;  ///< Used to exclude the source point if it's already in the tree.

    /// Axis-ordered point and hyperrectangle structure. Used internally to increase the cache hits.
    struct AxisData {
      T p; ///< Per-axis reference input point.
      T nearest; ///< Per-axis nearest point in the current hyperrectangle.
    } axis[D]; ///< Per-axis data defined this way to reduce cache misses.

    /// Initialize data for a tree search with incremental intersection calculation.
    SearchData(const Point &p, const Point *data, unsigned int K, bool ignore_p_in_tree);
  };

  /// Kd-tree leaf node.
  struct Leaf {
    unsigned int first_index; ///< Index of the first element contained by the leaf node.
    unsigned int num_elements; ///< Number of elements contained by the node.

    /// Leaf constructor.
    Leaf(unsigned int first_index, unsigned int num_elements) :
      first_index(first_index), num_elements(num_elements) {}

    /// Construct from an input stream.
    Leaf(std::istream &input);

    /// Process a leaf node with many buckets on it. Do not use any upper bounds on distance.
    template <typename C> void explore(SearchData &data, C &candidates) const;

    /// Process a leaf node with many buckets on it. Allows partial distance calculations.
    template <typename C> void intersect(SearchData &data, C &candidates) const;
    template <typename C> void intersect_ignoring_same(SearchData &data, C &candidates) const;

    /// Write to stream.
    bool write_to_binary_stream(std::ostream &out);

#ifdef DEBUG_KDTREE
    template <typename Op>
    bool verify_op(const Point* data, float value, int axis, const Op &op) const;
#endif
  };

  /// Kd-tree branch node.
  struct Node {

    union {
      Node *left_branch; ///< Left branch.
      Leaf *left_leaf; ///< Left leaf.
    };

    union {
      Node *right_branch; ///< Right branch.
      Leaf *right_leaf; ///< Right leaf.
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
    Node() : left_branch(NULL), right_branch(NULL), is_leaf(0) {} ///< Default constructor.
    Node(std::istream &input); ///< Construct from an input stream.
    ~Node(); ///< Default destructor.

#ifdef DEBUG_KDTREE
    bool verify(const Point* data, int axis) const;

    template <typename Op>
    bool verify_op(const Point* data, float value, int axis, const Op &op) const;
#endif

    /// Per axis element comparison functor. Used to apply STL sorting algorithms to individual axes.
    struct AxisComparer {
      const Point *data; ///< Array of input data.
      unsigned int axis; ///< Current axis used for sorting.

      /// Axis-th element comparison. Used to perform per-dimension data sorting.
      bool operator () (const unsigned int &i1, const unsigned int &i2) const {
        return data[i1][axis] < data[i2][axis];
      }
    };

    // --- Training-related --- //

    /// Build the kd-tree recursively.
    static Node *build(const Point *data, unsigned int *index, unsigned int n,
        Node *parent, unsigned int bucket_size, unsigned int &processed);

    /// Find a pivot to split the space in two by a chosen dimension during training.
    unsigned int split(unsigned int *index, unsigned int n, const AxisComparer &comparer);

    // --- Search-related --- //

    /// Traverse the kd-tree looking for nearest neighbours candidates based on Manhattan distances.
    template <typename C> void explore(const Node *parent, SearchData &data, C &candidates) const;

    /// Traverse the kd-tree checking hypersphere-hyperrectangle intersections to discard regions of the space.
    template <typename C> void intersect(const Node *parent, SearchData &data, C &candidates) const;

    // --- IO-related --- //

    /// Write to stream.
    bool write_to_binary_stream(std::ostream &out);
  };

  /// Incremental hyperrectangle-hypersphere intersection calculator. Designed so that the object lifespan handles increments.
  class Incremental {
  public:
    /// Create an temporary incremental calculator object. Will update the current data according with the selected branch.
    Incremental(const Node *node, const Node *parent, SearchData &data);

    /// Destroy a temporary incremental calculator object. Will restore any previous modifications of the incremental data.
    ~Incremental();

  protected:
    SearchData &search_data; ///< Reference to the search data being used.
    unsigned int parent_axis; ///< Axis that defines the hyperspace splitting.

    bool modified; ///< Flag indicating if the values were modified as part of the incremental update.
    T previous_axis_nearest; ///< Previous value of the local axis in the hyperrectangle.
    T previous_hyperrect_distance; ///< Previous value of the distance to the nearest point in the hyperrectangle.
  };

  // Internal methods.
  void release(); ///< Release memory allocated by the kd-tree including tree nodes.

  // Kd-tree data.
  Node *root; ///< Root node of the tree. Will be \c NULL in empty trees.
  Point *data; ///< Data of the kd-tree. Copied with element permutations during the tree building.
  unsigned int *permutation; ///< Permutation indices applied to kd-tree data to enhance cache behaviour.
  unsigned int *inverse_perm; ///< Inverse of the permutation applied to indices in the permutation array.
  unsigned int num_elements; ///< Number of elements currently in the tree.

  // File serialization settings.
  static const char *file_header; ///< Header written when serializing kd-trees.
  static const unsigned short file_header_length; ///< Length in bytes of the header.
  static const unsigned short file_version[2]; ///< Tuple of major and minor version of the current kd-tree file format.
  static const unsigned short signature; ///< Signature value used to check the end of file according to the format.
};

} // namespace kche_tree

// Template implementation.
#include "kd-tree.tpp"
#include "kd-tree_io.tpp"

#endif
