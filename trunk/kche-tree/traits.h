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

// Include exact-width integer types for serialization and type traits.
#ifdef KCHE_TREE_DISABLE_CPP0X
#include <tr1/cstdint>
#include <tr1/random>
#include <tr1/type_traits>
#else
#include <cstdint>
#include <random>
#include <type_traits>
#endif

// Demangling functions for the GNU C++ compiler.
#ifdef __GNUG__
#include <cxxabi.h>
#endif

// Include STL exceptions and numeric limits.
#include <stdexcept>
#include <limits>

// Include compile-time assertions, endianness operations, smart pointers and optimized params.
#include "compile_assert.h"
#include "endianness.h"
#include "utils.h"

namespace kche_tree {

// Bring to the local namespace the type traits from either TR1 or C++0x STL.
#ifdef KCHE_TREE_DISABLE_CPP0X
using std::tr1::is_fundamental;
using std::tr1::is_same;
using std::tr1::has_trivial_copy;
using std::tr1::is_base_of;
#else
using std::is_fundamental;
using std::is_same;
using std::has_trivial_copy_constructor;
using std::is_base_of;
#endif

/// Determine if a type is a fundamental one. This is basically an alias to the equivalent STL or TR1 type trait.
template <typename T>
struct IsFundamental : is_fundamental<T> {};

/**
 * \brief Specify if a given type uses a non-trivial equality operator to enable optimizations.
 *
 * This template allows the use of optimized operations when possible.
 * Any users of custom types that don't require non-trivial equality testing should specialize this to true.
 * An example of this can be found in the custom_type.cpp example.
 *
 * \note Any specialization setting \a value to \c true should also evaluate \c is_pot<T>::value to \c true.
 *       This is currently not enforced since some compilers do not provide this information yet.
 *
 * \tparam T Type which information is being provided.
 */
template <typename T>
struct HasTrivialEqual : IsFundamental<T> {};

/**
 * \brief Specify if a given type uses non-trivial stream operators to enable optimizations.
 *
 * This template allows the use of optimized operations when possible.
 * Any users of custom types that don't require non-trivial serialization testing should specialize this to true.
 * This concretely means that no <<, >> stream operators are implemented and that the data can be directly
 * read and written to/from memory as in a plain old data (POD) type.
 *
 * An example of this can be found in the custom_type.cpp example.
 *
 * \note Any specialization setting \a value to \c true should also evaluate \c is_pot<T>::value to \c true.
 *       This is currently not enforced since some compilers do not provide this information yet.
 *
 * \tparam T Type which information is being provided.
 */
template <typename T>
struct HasTrivialSerialization : IsFundamental<T> {};

// Forward-declare the templates provided here.
template <typename T> struct Traits;
template <typename T> struct AccumulationTraits;
template <typename T, bool isFundamental = IsFundamental<T>::value> struct NumericTraits;
template <typename T, bool hasTrivialEqual = HasTrivialEqual<T>::value> struct EqualTraits;
template <typename T, bool hasTrivialSerialization = HasTrivialSerialization<T>::value> struct SerializationTraits;

template <typename T, bool hasTrivialCopy =
#ifdef KCHE_TREE_DISABLE_CPP0X
  has_trivial_copy<T>::value
#else
  has_trivial_copy_constructor<T>::value
#endif
  > struct CopyTraits;

template <typename T, bool is_fundamental = IsFundamental<T>::value>
struct EndiannessTraits;

/**
 * \brief Specify the type that should be used when accumulating elements of type \a T.
 *
 * Provides a way to specifiy the accumulator that should be used when accumulating elements of type \a T.
 * Defaults to \a T itself.
 *
 * \warning Specializations of this trait won't work with the SSE-optimized metrics.
 * \tparam T Type which information is being provided.
 */
template <typename T>
struct AccumulationTraits {
  typedef T AccumulatorType; ///< Type that should be used when accumulating elements of type \a T.
};

/// Define the default random number engine to use.
typedef
    #ifdef KCHE_TREE_DISABLE_CPP0X
    std::tr1::minstd_rand
    #else
    std::default_random_engine
    #endif
    DefaultRandomEngine;

/**
 * \brief Provide the null and identity elements for the type \a T.
 * Especialization for fundamental values, returing 0 and 1.
 */
template <typename T>
struct NumericTraits<T, true> {
  static T max() { return std::numeric_limits<T>::max(); } ///< Return the maximum finite value of the type \a T.
  static T zero() { return T(); } ///< Returns the zero value.
  static T one() { return static_cast<T>(1.0); } ///< Returns the value one.
  static void negate(T &value) { value = -value; } ///< Replace \a value with its negative (additive inverse).
  static void invert(T &value) { value = one() / value; } ///< Replace \a value with its multiplicative inverse.

  typedef T AbsoluteValueType; ///< Type of the result returned by the absolute value function.
  static AbsoluteValueType abs(T value) { return fabs(value); } ///< Return the absolute value of \a value.

  /// Generate a random value using the provided generator.
  template <typename RandomGeneratorType>
  static T random(RandomGeneratorType &generator) { return generator(); }

  typedef T ExpectedDistributionElementType; ///< Defines the type of the elements generated by the random distribution provided on random number generation.
};

/**
 * \brief Provide the null and identity elements for the type \a T.
 * Especialization for custom types. The \c one method needs to be defined.
 */
template <typename T>
struct NumericTraits<T, false> {
  static T max() { std::numeric_limits<T>::max(); } ///< Return the maximum finite value of the type \a T. Needs to be defined for custom types.
  static T zero() { return T(); } ///< Returns the zero value or an object representing the null element for the type \a T.
  static T one(); ///< Returns the value one or an object representing the identity element for the type \a T. Needs to be defined for custom types that make use of the matrix calculations related to the Mahalanobis metric.
  static void negate(T &value); ///< Replace \a value with its negative (additive inverse). Needs to be defined for custom types that use matrix calculations related to the Mahalanobis metric.
  static void invert(T &value); ///< Replace \a value with its multiplicative inverse. Needs to be defined for custom types that use the matrix calculations related to the Mahalanobis metric and for making use of the verification tool.

  typedef T AbsoluteValueType; ///< Type of the result returned by the absolute value function. May be useful for custom types such as complex numbers.
  static AbsoluteValueType abs(const T &value); ///< Return the absolute value of \a value. Needs to be defined for custom types that make use of the verification tool.

  template <typename RandomGeneratorType>
  static T random(RandomGeneratorType &generator); ///< Generate a random value using the provided generator. Needs to be defined for custom types that make use of DataSet::init_to_random and the verification tool.

  typedef T ExpectedDistributionElementType; ///< Defines the type of the elements generated by the random distribution provided on random number generation. May be redefined by custom types that use random number generation. Otherwise STL template specializations are likely to be needed.
};

/// Provides optimized operations for types that have a trivial equality comparison.
template <typename T>
struct EqualTraits<T, true> {
  static bool equal_arrays(const T *p1, const T *p2, unsigned int size);
};

/// Provides standard operations for types that don't have a trivial equality comparison.
template <typename T>
struct EqualTraits<T, false> {
  static bool equal_arrays(const T *p1, const T *p2, unsigned int size);
};

/// Provides optimized operations for types that have a trivial copy.
template <typename T>
struct CopyTraits<T, true> {
  static void copy_array(T *dest, const T *source, unsigned int size);
};

/// Provides standard operations for types that don't have a trivial copy.
template <typename T>
struct CopyTraits<T, false> {
  static void copy_array(T *dest, const T *source, unsigned int size);
};

/// Provides serialization operations for types that have a trivial serialization.
template <typename T>
struct SerializationTraits<T, true> {
  static void serialize(const T &element, std::ostream &out);
  static void serialize_array(const T *array, unsigned int size, std::ostream &out);
  static void deserialize(T &element, std::istream &in, Endianness::Type endianness = Endianness::endianness());
  static void deserialize_array(T *array, unsigned int size, std::istream &in, Endianness::Type endianness = Endianness::endianness());
};


/// Provides serialization operations for types that don't have a trivial serialization.
template <typename T>
struct SerializationTraits<T, false> {
  static void serialize(const T &element, std::ostream &out);
  static void serialize_array(const T *array, unsigned int size, std::ostream &out);
  static void deserialize(T &element, std::istream &in, Endianness::Type endianness = Endianness::endianness());
  static void deserialize_array(T *array, unsigned int size, std::istream &in, Endianness::Type endianness = Endianness::endianness());
};

// Serialization functions.
template <typename T> void serialize(const T &element, std::ostream &out);
template <typename T> void serialize_array(const T *array, unsigned int size, std::ostream &out);
template <typename T, const size_t Size> void serialize(const T (&array)[Size], std::ostream &out);

// Deserialization functions.
template <typename T> void deserialize(T &element, std::istream &in, Endianness::Type endianness = Endianness::endianness());
template <typename T> void deserialize_array(T *array, unsigned int size, std::istream &in, Endianness::Type endianness = Endianness::endianness());
template <typename T, const size_t Size> void deserialize(T (&array)[Size], std::istream &in, Endianness::Type endianness = Endianness::endianness());

/// Provide mechanisms for type serialization and checking.
template <typename T>
struct TypeSerializationTraits {
  static void serialize_type(std::ostream &out);
  static void check_serialized_type(std::istream &in, Endianness::Type endianness = Endianness::endianness());
};

/// Provides functionality related to the name of the type being described.
template <typename T>
struct NameTraits {
  static const char *name();
};

/// Provides endianness swapping for fundamental types.
template <typename T>
struct EndiannessTraits<T, true> {
  static void swap_endianness(T &value); ///< Swap the endianness of the bytes in \a value.
};

/**
 * \brief Provides endianness swapping for non-fundamental types.
 * Custom types should add their specializations here.
 */
template <typename T>
struct EndiannessTraits<T, false> {
  static void swap_endianness(T &value); ///< Swap the endianness of the bytes in \a value. Needs to be defined by custom types that make use of serialization.
};

// Array endianness swapping function.
template <typename T, const size_t Size> void swap_endianness(T (&array)[Size]);

/**
 * \brief Provides specific information about the type \a T.
 *
 * \tparam T Type which information is being provided.
 */
template <typename T>
struct Traits : AccumulationTraits<T>,
                NumericTraits<T>,
                NameTraits<T>,
                CopyTraits<T>,
                EqualTraits<T>,
                SerializationTraits<T>,
                TypeSerializationTraits<T>,
                EndiannessTraits<T> {
  /// Type the traits are describing.
  typedef T Type;
};

} // namespace kche_tree

// Template implementation.
#include "traits.tpp"

#endif
