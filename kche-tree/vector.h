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

#ifndef _VECTOR_H_
#define _VECTOR_H_

// Includes from STL and the C standard library.
#include <cstdlib>
#include <functional>
#include <new>

namespace kche_tree {

/**
 * \brief Template for D-dimensional feature vectors.
 *
 * Encapsulates D-dimensional vectors containing feature values.
 * Has been compared to the direct use of arrays with no change in efficiency.
 *
 * \note For cache reasons it is recommended not to extend this class adding any labels
 * to the vectors, but to have separate label arrays and use the indices to access them.
 *
 * \tparam T Data type of the elements in the vector.
 * \tparam D Number of dimensions of the vector.
 */
template <typename T, const unsigned int D>
struct Vector {

  /// Data array.
  T data[D];

  // Constructors.
  Vector() {} ///< Default constructor.

  // Subscript operators.
  const T & operator [] (unsigned int index) const { return data[index]; } ///< Const subscript operator.
  T & operator [] (unsigned int index) { return data[index]; } ///< Subscript operator.

  // Comparison operators.
  bool operator == (const Vector &p) const; ///< Equality comparison operator.
  bool operator != (const Vector &p) const; ///< Non-equality comparison operator.

  // Squared distance operators for two D-dimensional points of type T.
  inline T distance_to(const Vector &p) const; ///< Squared distance to a point.
  inline T distance_to(const Vector &p, T upper_bound) const; ///< Squared distance to a point with an upper bound.

  // Memory operators: used to allow memory-aligned specializations. For example, for SSE optimizations.
  void *operator new [] (size_t size); ///< Standard allocation for arrays of feature vectors.
  void  operator delete [] (void *p); ///< Standard deallocation for arrays of feature vectors.

}
#ifdef __GNUC__
__attribute__((packed))
#endif
;

/**
 * \brief Vector-distance structure. References a feature vector by its index and its squared distance to another implicit vector.
 *
 * Implements its own comparison function with the parenthesis operator for STL-based algorithm use.
 *
 * \tparam T  Type used to encode the distance between two feature vectors. Should be the same than the data from the vectors.
 */
template <typename T>
struct VectorDistance : public std::binary_function <VectorDistance<T>, VectorDistance<T>, bool> {

  unsigned int index; ///< Index of the feature vector in the data set.
  T squared_distance; ///< Squared distance of the referenced element to an implicit point.

  // Default and convenience constructors.
  VectorDistance() {}
  VectorDistance(unsigned int index, T squared_distance) : index(index), squared_distance(squared_distance) {}

  /// Distance comparison operator for VectorDistances. Allows VectorDistance objects to be used as STL comparison functors.
  bool operator () (const VectorDistance &v1, const VectorDistance &v2) const {
    return v1.squared_distance < v2.squared_distance;
  }
};

} // namespace kche_tree

// Template implementation.
#include "vector.tpp"

#endif
