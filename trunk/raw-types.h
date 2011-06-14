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
 * \file raw-types.h
 * \brief Provide a way to identify types that could be operated with raw memory functions.
 * \author Leandro Graciá Gil
*/

#ifndef _RAW_TYPES_H_
#define _RAW_TYPES_H_

// C Standard Library and C++ STL includes.
#include <algorithm>
#include <cstring>
#include <limits>

namespace kche_tree {

/**
 * \brief Define if a specific type can be managed by its raw data only.
 *
 * Tells the library to use C string functions to copy and compare data instead of the std templates.
 * If enabled, it may provide higher efficiency to raw data types like integers or floating point values
 * since the optimizations for STL are implementation-specific.
 *
 * All numeric types are enabled by default. Other types should be enabled only if they are plain data holders
 * that could be copied without any extra operations. To enable this behaviour to them just specialize this method.
 *
 * \warning This excludes any types holding dynamically allocated data and extreme care is suggested when
 * providing types that use virtual methods or multiple inheritance in any point of their hierarchy.
 * Keep in mind that memcpy will copy and potentially overwrite your virtual pointer table.
 */
template <typename T>
bool is_raw_type() { return std::numeric_limits<T>::is_specialized; }

/**
 * \brief Copy an array of elements knowing if they belong to a raw type.
 *
 * Copy an array of elements using the raw type information provided by \link kche_tree::is_raw_type is_raw_type \endlink.
 * \param dest Destination address.
 * \param source Source address.
 * \param size Number of elements to copy.
 * \tparam T Type of the elements to copy.
 */
template <typename T>
void copy_array(T *dest, const T *source, unsigned int size) {
  if (is_raw_type<T>())
    memcpy(dest, source, size * sizeof(T));
  else
    std::copy(source, source + size, dest);
}

/**
 * \brief Determine if two arrays of elements are equal knowing if they belong to a raw type.
 *
 * Compare two arrays to determine if they are equal using the raw type information provided by \link kche_tree::is_raw_type is_raw_type \endlink.
 * \tparam T Type of the elements to compare.
 * \param p1 First array.
 * \param p2 Second array.
 * \param size Number of elements in both arrays.
 * \return \c true if the arrays are equal, \c false otherwise.
 */
template <typename T>
bool equal_arrays(const T *p1, const T *p2, unsigned int size) {
  if (is_raw_type<T>())
    return memcmp(p1, p2, size * sizeof(T)) == 0;
  return std::equal(p1, p1 + size, p2);
}

} // namespace kche_tree

#endif
