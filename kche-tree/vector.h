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
 * \file vector.h
 * \brief Template for generic D-dimensional feature vectors.
 * \author Leandro Graciá Gil
*/

#ifndef _KCHE_TREE_VECTOR_H_
#define _KCHE_TREE_VECTOR_H_

namespace kche_tree {

/**
 * \brief Template for D-dimensional feature vectors.
 *
 * Encapsulates D-dimensional contiguous vectors containing feature values.
 * Has been compared to the direct use of arrays with no change in efficiency.
 *
 * \note For cache reasons it is recommended not to extend this class adding any labels
 * to the vectors, but to have separate label arrays and use the indices to access them.
 *
 * \tparam T Data type of the elements in the vector.
 * \tparam D Number of dimensions of the vector.
 */
template <typename T, const unsigned int D>
class Vector {
public:
  /// Type of the elements in the vector.
  typedef T ElementType;

  /// Number of dimensions (size) of the vector.
  static const unsigned int Dimensions = D;

  // Direct access to the data array.
  const T *data() const { return data_; }

  // Subscript operators.
  const T & operator [] (unsigned int index) const { return data_[index]; } ///< Const subscript operator.
  T & operator [] (unsigned int index) { return data_[index]; } ///< Subscript operator.

  // Comparison operators.
  bool operator == (const Vector &p) const; ///< Equality comparison operator. May be optimized if \link kche_tree::has_trivial_equal has_trivial_equal::value\endlink is \c true.
  bool operator != (const Vector &p) const; ///< Non-equality comparison operator. May be optimized if \link kche_tree::has_trivial_equal has_trivial_equal::value\endlink is \c true.

  // Memory operators: used to allow memory-aligned specializations. For example, for SSE optimizations.
  void *operator new [] (size_t size); ///< Standard allocation for arrays of feature vectors.
  void  operator delete [] (void *p); ///< Standard deallocation for arrays of feature vectors.

private:
  /// Contiguous D-dimensional data array.
  T data_[D];
};

/**
 * \brief References a feature vector by its index in the data set and provides the squared distance to it from an implicit vector.
 *
 * Implements its own comparison function with the parenthesis operator for STL-based algorithm use.
 *
 * \tparam T Type used to encode the distance between two feature vectors. Should be the same than the data from the vectors.
 */
template <typename T>
struct VectorDistance : public std::binary_function<VectorDistance<T>, VectorDistance<T>, bool> {

  unsigned int index; ///< Index of the feature vector in the data set.
  T squared_distance; ///< Squared distance of the referenced element to an implicit vector.

  // Default and convenience constructors.
  VectorDistance() {}
  VectorDistance(unsigned int index, const T &squared_distance) : index(index), squared_distance(squared_distance) {}

  /// Distance comparison operator for VectorDistances. Allows VectorDistance objects to be used as STL comparison functors.
  bool operator () (const VectorDistance &v1, const VectorDistance &v2) const {
    return v1.squared_distance < v2.squared_distance;
  }
};

} // namespace kche_tree

// Template implementation.
#include "vector.tpp"

#endif
