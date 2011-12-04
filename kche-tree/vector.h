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

#include "endianness.h"
#include "sse.h"
#include "traits.h"
#include "utils.h"

namespace kche_tree {

/**
 * \brief Stream serialization operator.
 *
 * \note This method does not perform any type checking and it's used internally when
 * serializing data sets. For proper vector serialization, use DataSet objects.
 * \note Data is serialized with the same endianness as the local host.
 *
 * \param out Output stream.
 * \param vector Vector to be serialized.
 * \exception std::runtime_error Thrown in case of write error.
 */
template <typename T, const unsigned int D>
std::ostream & operator << (std::ostream &out, const Vector<T, D> &vector);

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

  // Default constructor.
  Vector();

  // Read the vector from an input stream.
  Vector(std::istream &in, Endianness::Type endianness);

  // Direct access to the data array.
  const T *data() const { return data_; }
  T *mutable_data() { return data_; }

  // Subscript operators.
  const T & operator [] (unsigned int index) const { return data_[index]; } ///< Const subscript operator.
  T & operator [] (unsigned int index) { return data_[index]; } ///< Subscript operator.

  // Comparison operators.
  bool operator == (const Vector &p) const; ///< Equality comparison operator. May be optimized if \link kche_tree::HasTrivialEqual HasTrivialEqual::value\endlink is \c true.
  bool operator != (const Vector &p) const; ///< Non-equality comparison operator. May be optimized if \link kche_tree::HasTrivialEqual HasTrivialEqual::value\endlink is \c true.

  // Friends for endianness swapping and serialization.
  friend struct EndiannessTraits<Vector<T, D> >;
  friend std::ostream & operator << <>(std::ostream &out, const Vector &vector);

  // Memory operators to handle automatic alignment in SSE.
  void *operator new (size_t nbytes);
  void *operator new [] (size_t nbytes);
  void operator delete (void *p);
  void operator delete [] (void *p);

private:
  /// Contiguous D-dimensional data array extended to the next multiple of 4 when SSE is enabled.
  /// Extra data is initialized to zero using the appropriate method in traits.
  T data_[KCHE_TREE_SSE_COMPILE_ALIGN(T, D)];
}
#if KCHE_TREE_ENABLE_SSE
KCHE_TREE_ALIGNED(16)
#endif
;

/// Vectors have trivial serialization if their contents do.
template <typename T, const unsigned int D>
struct HasTrivialSerialization<Vector<T, D> > {
  static const bool value = HasTrivialSerialization<T>::value;
};

/**
 * \brief Enable trivial equality comparison for vectors if their contents do.
 *
 * Vectors do implement the equality comparison operator, but only in order
 * to perform raw-memory optimizations over all their dimensions.
 *
 * Since Vectors are POD objects, it is safe to set them trivial to compare
 * if their contents are. This allows DataSets to compare arrays of vectors
 * using raw memory comparisons if possible.
 */
template <typename T, const unsigned int D>
struct HasTrivialEqual<Vector<T, D> > {
  static const bool value = HasTrivialEqual<T>::value;
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

  /// Use optimized const reference types.
  typedef typename RParam<T>::Type ConstRef_T;

  unsigned int index; ///< Index of the feature vector in the data set.
  T squared_distance; ///< Squared distance of the referenced element to an implicit vector.

  // Default and convenience constructors.
  VectorDistance() {}
  VectorDistance(unsigned int index, ConstRef_T squared_distance) : index(index), squared_distance(squared_distance) {}

  /// Distance comparison operator for VectorDistances. Allows VectorDistance objects to be used as STL comparison functors.
  bool operator () (const VectorDistance &v1, const VectorDistance &v2) const {
    return v1.squared_distance < v2.squared_distance;
  }
};

} // namespace kche_tree

// Template implementation.
#include "vector.tpp"

#endif
