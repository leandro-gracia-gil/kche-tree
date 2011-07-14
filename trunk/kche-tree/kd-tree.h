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

// Include STL streams and vectors for K neighbours output.
#include <iostream>
#include <vector>

// Include k-neighbours containers (k-heaps and k-vectors).
#include "k-heap.h"
#include "k-vector.h"

// Include data sets, vectors, type traits, kd-tree nodes, metrics and optimized parameters.
#include "dataset.h"
#include "vector.h"
#include "traits.h"
#include "kd-node.h"
#include "metrics.h"
#include "rparam.h"

namespace kche_tree {

// Forward-declare the class.
template <typename T, const unsigned int D> class KDTree;

/**
 * \brief Standard input stream operator. Loads the kd-tree from a stream in binary format.
 *
 * The original contents of the \a kdtree object are not modified in case of error.
 *
 * \param in Input stream.
 * \param kdtree Kd-tree where results will be saved.
 * \return Input stream after reading the data.
 * \exception std::runtime_error Thrown in case of error reading or processing the kd-tree.
 */
template <typename T, const unsigned int D>
std::istream & operator >> (std::istream &in, KDTree<T, D> &kdtree);

/**
 * \brief Standard output stream operator.
 *
 * Saves the kd-tree into a stream in binary format.
 * \note Data is serialized with the same endianness as the local host.
 *
 * \param out Output stream.
 * \param kdtree Kd-tree being saved.
 * \return Output stream after writting the data.
 * \exception std::runtime_error Thrown in case of error writing the kd-tree.
 */
template <typename T, const unsigned int D>
std::ostream & operator << (std::ostream &out, const KDTree<T, D> &kdtree);

/**
 * \brief Template for generic cache-aware kd-trees of any type.
 *
 * Element insertions and deletions are not currently supported in behalf of a design based on cache efficiency.
 * \warning This class is currently not thread-safe. This feature will be added in future releases.
 *
 * \tparam T Data type of the elements in the kd-tree. Requires copy construction, asignment and the < <= > >= == operators. Further operators may be required by the metrics used. Check the documentation for EuclideanMetric if using the default metric values. For serialization the << operator is required, as well as the T(std::ifstream &, Endianness::Type) constructor for reading custom objects with the correct byte endianness. See the \c custom_type example for details.
 * \tparam D Number of dimensions in the input data.
 */
template <typename T, const unsigned int D>
class KDTree {
public:
  /// Type of the elements in the kd-tree.
  typedef T ElementType;

  /// Define the number of dimensions of the kd-tree.
  static unsigned const int Dimensions = D;

  /// Type used for kd-tree neighbours.
  typedef VectorDistance<T> NeighbourType;

  /// Use the global data set type by default.
  typedef typename Settings<T, D>::DataSetType DataSetType;

  /// Use the global vector type by default.
  typedef typename Settings<T, D>::VectorType VectorType;

  /// Use optimized const reference types.
  typedef typename RParam<T>::Type ConstRef_T;

  /// Default constructor. Creates an empty and uninitialized kd-tree.
  KDTree() {}

  // Basic kd-tree operations.
  bool build(const DataSetType& train_set, unsigned int bucket_size = 32); ///< Build a kd-tree from a set of training vectors. Cost: O(n log² n).

  #ifdef KCHE_TREE_DISABLE_CPP0X
  template <template <typename, typename Compare> class KContainer, typename M>
  void knn(const VectorType &p, unsigned int K, std::vector<NeighbourType> &output, const M &metric = EuclideanMetric<T, D>(), ConstRef_T epsilon = Traits<T>::zero(), bool ignore_p_in_tree = false) const; ///< Get the K nearest neighbours of a point. Estimated average cost: O(log K log n).
  #else
  template <template <typename, typename Compare> class KContainer = KVector, typename M = EuclideanMetric<T, D> >
  void knn(const VectorType &p, unsigned int K, std::vector<NeighbourType> &output, const M &metric = M(), ConstRef_T epsilon = Traits<T>::zero(), bool ignore_p_in_tree = false) const; ///< Get the K nearest neighbours of a point. Estimated average cost: O(log K log n).
  #endif

  #ifdef KCHE_TREE_DISABLE_CPP0X
  template <typename M>
  void all_in_range(const VectorType &p, ConstRef_T distance, std::vector<NeighbourType> &output, const M &metric = EuclideanMetric<T, D>(), bool ignore_p_in_tree = false) const; ///< Get all neighbours within a distance from a point. Estimated average Cost: O(log m log n) depending on the number of results m.
  #else
  template <typename M = EuclideanMetric<T, D> >
  void all_in_range(const VectorType &p, ConstRef_T distance, std::vector<NeighbourType> &output, const M &metric = M(), bool ignore_p_in_tree = false) const; ///< Get all neighbours within a distance from a point. Estimated average Cost: O(log m log n) depending on the number of results m.
  #endif

  /// Subscript operator for accesing the original train set data. Will throw an exception if \a index is out of bounds.
  const VectorType &operator [] (unsigned int index) const;

  // Stream operators.
  friend std::istream &operator >> <>(std::istream &in, KDTree &kdtree);
  friend std::ostream &operator << <>(std::ostream &out, const KDTree &kdtree);

  // Kd-tree properties.
  unsigned int size() const { return data.size(); } ///< Get the number of elements stored in the tree.

private:
  // Kd-tree data.
  ScopedPtr<KDNode<T, D> > root; ///< Root node of the tree. Will be \c NULL in empty trees.
  DataSet<T, D> data; ///< Data of the kd-tree. Consists of a permutated version of the training set created while building the tree.
  ScopedArray<uint32_t> permutation; ///< Permutation indices applied to kd-tree data to enhance cache behaviour.
  ScopedArray<uint32_t> inverse_permutation; ///< Inverse of the permutation applied to indices in the permutation array.

  // Serialization settings.
  static const uint16_t version[2]; ///< Tuple of major and minor version of the current kd-tree serialization format.
  static const uint16_t signature; ///< Signature value used to check the end of data according to the current format.
};

} // namespace kche_tree

// Template implementation.
#include "kd-tree.tpp"
#include "kd-tree_io.tpp"

#endif
