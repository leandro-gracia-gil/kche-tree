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
 * \file vector.tpp
 * \brief Template implementations for generic D-dimensional feature vectors.
 * \author Leandro Graciá Gil
 */

#include "allocator.h"
#include "traits.h"

namespace kche_tree {

/**
 * \brief Initialize an empty vector.
 *
 * If SSE is enabled, there may be some additional elements initialized to zero at the end of the array.
 */
template <typename T, const unsigned int D>
Vector<T, D>::Vector() {
  initSSEAlignmentGap(data_, D);
}

/**
 * Check if two feature vectors are exactly equal.
 *
 * \param p Feature vector being compared to.
 * \return \c true if equal, \c false otherwise.
 */
template <typename T, const unsigned int D>
bool Vector<T, D>::operator == (const Vector &p) const {
  return Traits<T>::equal_arrays(data_, p.data_, D);
}

/**
 * Check if two feature vectors are different equal.
 *
 * \param p Feature vector being compared to.
 * \return \c true if different, \c false otherwise.
 */
template <typename T, const unsigned int D>
bool Vector<T, D>::operator != (const Vector &p) const {
  return !Traits<T>::equal_arrays(data_, p.data_, D);
}

/// Overloaded new operator to support memory alignment if required.
template <typename T, const unsigned int D>
void *Vector<T, D>::operator new (size_t nbytes) {
  return Allocator<>::alloc(nbytes);
}

/// Overloaded new [] operator to support memory alignment if required.
template <typename T, const unsigned int D>
void *Vector<T, D>::operator new [] (size_t nbytes) {
  return Allocator<>::alloc(nbytes);
}

/// Overloaded delete operator to properly delete aligned memory.
template <typename T, const unsigned int D>
void Vector<T, D>::operator delete (void *p) {
  Allocator<>::dealloc(p);
}

/// Overloaded delete [] operator to properly delete aligned memory.
template <typename T, const unsigned int D>
void Vector<T, D>::operator delete [] (void *p) {
  Allocator<>::dealloc(p);
}

/**
 * \brief Stream deserialization constructor.
 * Any required endianness correction is performed when reading the data.
 *
 * \note This method does not perform any type checking and it's used internally when
 * serializing data sets. For proper vector serialization, use DataSet objects.
 *
 * \param in Input stream.
 * \param endianness Endianness of the serialized data.
 * \exception std::runtime_error Thrown in case of read or validation error.
 */
template <typename T, const unsigned int D>
Vector<T, D>::Vector(std::istream &in, Endianness::Type endianness) {
  kche_tree::deserialize(data_, in, endianness);
  initSSEAlignmentGap(data_, D);
}


/**
 * \brief Stream serialization operator.
 *
 * \param out Output stream.
 * \exception std::runtime_error Thrown in case of write error.
 */
template <typename T, const unsigned int D>
void Vector<T, D>::serialize(std::ostream &out) const {
  kche_tree::serialize(data_, out);
}

/**
 * \brief Swap the contents of the vector with some other.
 *
 * Used as part of the deserialization process.
 *
 * \param vector Vector to swap contents with.
 */
template <typename T, unsigned int D>
void Vector<T, D>::swap(Vector &vector) {
  std::swap(data_, vector.data_);
}

/**
 * \brief Define the endianness swap for Vectors based on its element array.
 *
 * The \c false argument in the template comes from the fact that kche-tree Vectors
 * are not fundamental C++ types. It should be deduced by default, but leads
 * to errors in some compilers if not specified.
 */
template <typename T, const unsigned int D>
struct EndiannessTraits<Vector<T, D>, false> {
  static const void swap_endianness(Vector<T, D> &value) {
    kche_tree::swap_endianness(value.data_);
  }
};

} // namespace kche_tree
