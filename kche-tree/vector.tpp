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

// Includes from STL and the C standard library.
#include <cstdlib>
#include <functional>

// Include the map-reduce metaprograming templates and traits.
#include "map_reduce.h"
#include "traits.h"

namespace kche_tree {

/**
 * Generic memory allocator operator for feature vector arrays.
 * Defined so that memory-aligned specializations can be defined if required.
 *
 * \param size Total size in bytes of the objects to allocate.
 * \return Address to the new allocated memory.
 */
template <typename T, const unsigned int D>
void *Vector<T, D>::operator new [] (size_t size) {

  // Allocate plain memory for the requested vectors.
  void *p = malloc(size);

  // Throw an allocation exception in case of error.
  if (!p)
    throw std::bad_alloc();

  return p;
}

/**
 * Generic memory deallocator operator for feature vector arrays.
 * Defined as complement of operator new [] so that memory-aligned specializations can be defined if required.
 *
 * \param p Pointer to the address to release.
 */
template <typename T, const unsigned int D>
void Vector<T, D>::operator delete [] (void *p) {

  // Just release memory.
  free(p);
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
  deserialize(data_, in, endianness);
}

/**
 * \brief Stream serialization operator.
 *
 * \note This method does not perform any type checking and it's used internally when
 * serializing data sets. For proper vector serialization, use DataSet objects.
 *
 * \param out Output stream.
 * \param vector Vector to be serialized.
 * \exception std::runtime_error Thrown in case of write error.
 */
template <typename T, const unsigned int D>
std::ostream& operator << (std::ostream& out, const Vector<T, D> &vector) {
  serialize(vector.data_, out);
  return out;
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
