/***************************************************************************
 *   Copyright (C) 2010, 2011, 2012 by Leandro Graciá Gil                  *
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
 * \brief Template for D-dimensional feature vectors.
 *
 * Encapsulates D-dimensional contiguous vectors containing feature values.
 * Has been compared to the direct use of arrays with no change in efficiency.
 *
 * \note For cache reasons it is recommended not to extend this class adding any labels
 * to the vectors, but to have separate label arrays and use the indices to access them.
 *
 * \tparam ElementType Type of the elements in the vector.
 * \tparam NumDimensions Number of dimensions of the vector.
 */
template <typename ElementType, const unsigned int NumDimensions>
class Vector : public Serializable<Vector<ElementType, NumDimensions> > {
public:
  /// Type of the elements in the vector.
  typedef ElementType Element;

  /// Number of dimensions (size) of the vector.
  static const unsigned int Dimensions = NumDimensions;

  // Default constructor.
  Vector();

  // Direct access to the data array.
  const Element* data() const { return data_; }
  Element* mutable_data() { return data_; }

  // Subscript operators.
  const Element& operator [] (unsigned int index) const { return data_[index]; } ///< Const subscript operator.
  Element& operator [] (unsigned int index) { return data_[index]; } ///< Subscript operator.

  // Comparison operators.
  bool operator == (const Vector &p) const; ///< Equality comparison operator. May be optimized if \link kche_tree::HasTrivialEqual HasTrivialEqual::value\endlink is \c true.
  bool operator != (const Vector &p) const; ///< Non-equality comparison operator. May be optimized if \link kche_tree::HasTrivialEqual HasTrivialEqual::value\endlink is \c true.

  // Memory operators to handle automatic alignment in SSE.
  void* operator new (size_t nbytes);
  void* operator new [] (size_t nbytes);
  void operator delete (void *p);
  void operator delete [] (void *p);

private:
  // Implementation of the serializable concept.
  Vector(std::istream &in, Endianness::Type endianness);
  void serialize(std::ostream &out) const;
  void swap(Vector &vector);

  // Friends for endianness swapping and serialization.
  friend struct EndiannessTraits<Vector>;
  friend std::istream& operator >> <>(std::istream &in, Serializable<Vector> &vector);
  friend std::ostream& operator << <>(std::ostream &out, const Serializable<Vector> &vector);

  /// Contiguous D-dimensional data array extended to the next multiple of 4 when SSE is enabled.
  /// Extra data is initialized to zero using the appropriate method in traits.
  Element data_[KCHE_TREE_SSE_COMPILE_ALIGN(Element, Dimensions)];
}
#if KCHE_TREE_ENABLE_SSE
KCHE_TREE_ALIGNED(16)
#endif
;

/**
 * \brief Vectors have trivial serialization if their contents do.
 *
 * The serializable interface will be only used for custom types.
 */
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

} // namespace kche_tree

// Template implementation.
#include "vector.tpp"

#endif
