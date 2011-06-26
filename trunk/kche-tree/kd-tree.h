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

// Include data sets, vectors, type traits and kd-tree nodes.
#include "dataset.h"
#include "vector.h"
#include "traits.h"
#include "kd-node.h"

/// Namespace of the Kche-tree template library.
namespace kche_tree {

// Forward-declare the class.
template <typename T, const unsigned int D, typename S> class KDTree;

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
  void knn(const Point &p, unsigned int K, std::vector<Neighbour> &output, const T &epsilon = Traits<T>::zero(), bool ignore_p_in_tree = false) const; ///< Get the K nearest neighbours of a point. Estimated average cost: O(log K log n).
  void all_in_range(const Point &p, const T &distance, std::vector<Neighbour> &output, bool ignore_p_in_tree = false) const; ///< Get all neighbours within a distance from a point. Estimated average Cost: O(log m log n) depending on the number of results m.

  /// Subscript operator for accesing stored data (will fail on non-built kd-trees).
  const Point &operator [] (unsigned int index) const;

  // Stream operators.
  friend std::istream &operator >> <>(std::istream &in, KDTree &kdtree);
  friend std::ostream &operator << <>(std::ostream &out, const KDTree &kdtree);

  // Kd-tree properties.
  unsigned int size() const { return num_elements; } ///< Get the number of elements stored in the tree.

#ifdef _KCHE_TREE_DEBUG_
  bool verify() const;
#endif

private:
  // Internal methods.
  void release(); ///< Release memory allocated by the kd-tree including tree nodes.

  // Kd-tree data.
  KDNode<T, D> *root; ///< Root node of the tree. Will be \c NULL in empty trees.
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
