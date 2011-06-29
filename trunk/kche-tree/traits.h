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
 * \file traits.h
 * \brief Template definitions for type traits and their basic operations.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_TRAITS_H_
#define _KCHE_TREE_TRAITS_H_

#ifdef KCHE_TREE_DISABLE_CPP0X
#include <tr1/type_traits>
#else
#include <type_traits>
#endif

#include "compile_assert.h"

namespace kche_tree {

/**
 * \brief Specify if a given type uses a non-trivial equality operator to enable optimizations.
 *
 * This template allows the use of optimized operations when possible.
 * Any users of custom types that don't require non-trivial equality testing should specialize this to true.
 * An example of this can be found in the custom_type.cpp example.
 *
 * This mimics the structure of STL TR1 type traits, since no trait is provided for this information.
 * \note Any specialization setting the value to \c true should naturally evaluate std::tr1::is_pot<T>::value to \c true.
 *       This is currently not enforced since some compilers do not provide this information yet.
 *
 * \tparam T Type which information is being provided.
 */
template <typename T>
struct has_trivial_equal :
#ifdef KCHE_TREE_DISABLE_CPP0X
    std::tr1::integral_constant<bool, std::tr1::is_fundamental<T>::value>
#else
    std::integral_constant<bool, std::is_fundamental<T>::value>
#endif
    {};

// Forward-declare the templates provided here.
template <typename T> struct Traits;
template <typename T> struct AccumulationTraits;
template <typename T, bool hasTrivialEqual> struct EqualTraits;
template <typename T, bool hasTrivialCopy> struct CopyTraits;

/**
 * \brief Base of the traits information.
 *
 * \tparam T Type which information is being provided.
 */
template <typename T>
struct TraitsBase {
  typedef T Type; ///< Type the traits are describing.
  static T zero() { return T(); } ///< Creates a zero value or empty object for the type \a T.
};

/**
 * \brief Specify the type that should be used when accumulating elements of type \a T.
 *
 * Provides a way to specifiy the accumulator that should be used when accumulating elements of type \a T.
 * Defaults to \a T itself.
 *
 * \warning This trait will be ignored by SSE-optimized metrics.
 * \tparam T Type which information is being provided.
 */
template <typename T>
struct AccumulationTraits {
  // In case we need more precision, this should be specialized with a promotion of the T type.
  typedef T AccumulatorType; ///< Type that should be used when accumulating elements of type \a T.
};

/**
 * \brief Provides optimized operations for types that have a trivial equality comparison.
 */
template <typename T>
struct EqualTraits<T, true> {
  /**
   * Compare the contents of two arrays of the given size.
   *
   * \param p1 Pointer to the first array.
   * \param p2 Pointer to the second array.
   * \param size Size of both arrays.
   * \return \c true if equal, \c false otherwise.
   */
  static bool equal_arrays(const T *p1, const T *p2, unsigned int size) {
    return memcmp(p1, p2, size * sizeof(T)) == 0;
  }
};

/**
 * \brief Provides standard operations for types that don't have a trivial equality comparison.
 */
template <typename T>
struct EqualTraits<T, false> {
  /**
   * Compare the contents of two arrays of the given size.
   *
   * \param p1 Pointer to the first array.
   * \param p2 Pointer to the second array.
   * \param size Size of both arrays.
   * \return \c true if equal, \c false otherwise.
   */
  static bool equal_arrays(const T *p1, const T *p2, unsigned int size) {
    return std::equal(p1, p1 + size, p2);
  }
};

/**
 * \brief Provides optimized operations for types that have a trivial copy.
 */
template <typename T>
struct CopyTraits<T, true> {
  /**
   * Copy the contents of one array to another.
   * Arrays are assumed to not overlap.
   *
   * \param dest Pointer to the destination array.
   * \param source Pointer to the source array.
   * \param size Size of both arrays.
   */
  static void copy_array(T *dest, const T *source, unsigned int size) {
    memcpy(dest, source, size * sizeof(T));
  }
};

/**
 * \brief Provides standard operations for types that don't have a trivial copy.
 */
template <typename T>
struct CopyTraits<T, false> {
  /**
   * Copy the contents of one array to another.
   * Arrays are assumed to not overlap.
   *
   * \param dest Pointer to the destination array.
   * \param source Pointer to the source array.
   * \param size Size of both arrays.
   */
  static void copy_array(T *dest, const T *source, unsigned int size) {
    std::copy(source, source + size, dest);
  }
};

/**
 * \brief Provides specific information about the type \a T.
 *
 * \tparam T Type which information is being provided.
 */
template <typename T>
struct Traits : TraitsBase<T>,
                AccumulationTraits<T>,
                #ifdef KCHE_TREE_DISABLE_CPP0X
                CopyTraits<T, std::tr1::has_trivial_copy<T>::value>,
                #else
                CopyTraits<T, std::has_trivial_copy_constructor<T>::value>,
                #endif
                EqualTraits<T, has_trivial_equal<T>::value> {};

// Bring to the kche_tree namespace the appropriate is_same template.
using
#ifdef KCHE_TREE_DISABLE_CPP0X
    std::tr1::is_same;
#else
    std::is_same;
#endif

} // namespace kche_tree

#endif
