/***************************************************************************
 *   Copyright (C) 2011, 2012 by Leandro Graciá Gil                        *
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

// Demangling functions for the GNU C++ compiler.
#ifdef __GNUG__
#include <cxxabi.h>
#endif

#include <iterator>
#include <limits>
#include <stdexcept>

#include "cpp1x.h"
#include "endianness.h"
#include "utils.h"

namespace kche_tree {

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
template <typename T, bool is_arithmetic = IsArithmetic<T>::value> struct NumericTraits;
template <typename T, bool is_arithmetic = IsArithmetic<T>::value> struct DistanceNumericTraits;
template <typename T, bool is_fundamental = IsFundamental<T>::value> struct RandomGenerationTraits;
template <typename T, bool has_trivial_equal = HasTrivialEqual<T>::value> struct EqualTraits;
template <typename T, bool has_trivial_copy = HasTrivialCopyConstructor<T>::value> struct CopyTraits;
template <typename T, bool has_trivial_serialization = HasTrivialSerialization<T>::value> struct SerializationTraits;
template <typename T, bool is_fundamental = IsFundamental<T>::value> struct EndiannessTraits;

/**
 * \brief Basic numeric traits and operations for fundamental types.
 *
 * Provides the basic operations and types, including how to encode distances and calculate them from a pair of elements.
 */
template <typename T>
struct NumericTraits<T, true> {
  /// Type used to represent distances between elements of type \a T.
  typedef T Distance;

  /// Returns the distance between two given elements.
  static Distance distance(typename RParam<T>::Type a, typename RParam<T>::Type b) { return a - b; }

  /// Returns the zero value or an object representing the mathematical null element for the type \a T.
  static T zero() { return T(); }

  /// Return the mean element from a list of elements provided by an iterator.
  template <typename BidirectionalElementIterator>
  static T mean(const BidirectionalElementIterator &begin, const BidirectionalElementIterator &end) {
    T acc = Traits<T>::zero();
    unsigned int n = 0;
    for (BidirectionalElementIterator it = begin; it != end; ++it) {
      acc += *it;
      ++n;
    }
    return acc /= n;
  }
};

/**
 * \brief Basic numeric traits and operations for custom types.
 *
 * Provides the basic operations and types, including how to encode distances and calculate them from a pair of elements.
 */
template <typename T>
struct NumericTraits<T, false> {
  /// Type used to represent distances between elements of type \a T. Defaults to float and may be redefined for custom types.
  typedef float Distance;

  /// Returns the distance between two given elements. Should be redefined for custom types.
  static Distance distance(typename RParam<T>::Type a, typename RParam<T>::Type b);

  /// Return the mean value from an array of elements. Needs to be defined for custom types that use set_inverse_covariance(DataSet) method of the Mahalanobis metric object.
  template <typename BidirectionalElementIterator>
  static T mean(const BidirectionalElementIterator &begin, const BidirectionalElementIterator &end);
};

/// Traits required for types encoding distances in fundamental types.
template <typename T>
struct DistanceNumericTraits<T, true> {
  /// Returns the value one.
  static T one() { return static_cast<T>(1.0); }

  /// Return the maximum finite value of the type \a T. Used by the verification tool only.
  static T max() { return std::numeric_limits<T>::max(); }

  /// Replace \a value with its negative (additive inverse).
  static void negate(T &value) { value = -value; }

  /// Replace \a value with its multiplicative inverse.
  static void invert(T &value) { value = one() / value; }

  /// Type of the result returned by the absolute value function.
  typedef T AbsoluteValue;

  /// Return the absolute value of \a value.
  static AbsoluteValue abs(T value) { return fabs(value); }
};

/// Traits required for types encoding distances in custom types.
template <typename T>
struct DistanceNumericTraits<T, false> {
  /// Returns the value one or an object representing the identity element for the type \a T. Needs to be defined for custom distance types that make use of the matrix calculations related to the Mahalanobis metric.
  static T one();

  /// Return the maximum finite value of the type \a T. Needs to be defined custom types making use of the verification tool.
  static T max();

  /// Replace \a value with its negative (additive inverse). Needs to be defined for custom distance types that use matrix calculations related to the Mahalanobis metric.
  static void negate(T &value);

  /// Replace \a value with its multiplicative inverse. Needs to be defined for custom distance types that use the matrix calculations related to the Mahalanobis metric and for making use of the verification tool.
  static void invert(T &value);

  /// Type of the result returned by the absolute value function. May be useful for custom distance types such as complex numbers.
  typedef T AbsoluteValue;

  /// Return the absolute value of \a value. Needs to be defined for custom distance types that make use of the verification tool.
  static AbsoluteValue abs(const T &value);
};

/// Default random distributions to use for a type. May be redefined by custom types in order to provide integer or floating point versions as appropriate.
template <typename T>
struct DefaultRandomDistribution {
  /// Type of the corresponding uniform distribution. Defaults to UniformInt for integral values and UniformReal otherwise.
  typedef typename TypeBranch<IsIntegral<T>::value, UniformInt<T>, UniformReal<T> >::Result UniformDistribution;
};

/// Traits for generating random elements of arithmetic types.
template <typename T>
struct RandomGenerationTraits<T, true> {
  /// Type of the random distribution elements. Defines the output type of the random generator argument that the random method uses to create elements.
  typedef T RandomDistributionElement;

  /// Type of the uniform distribution used in the generated expected by the random method.
  typedef typename DefaultRandomDistribution<RandomDistributionElement>::UniformDistribution UniformDistribution;

  /// Generate a random value using the provided generator.
  template <typename RandomGenerator>
  static T random(RandomGenerator &generator) { return generator(); }
};

/// Traits for generating random elements of custom types.
template <typename T>
struct RandomGenerationTraits<T, false> {
  /// Type of the random distribution elements. Defines the output type of the random generator argument that the random method uses to create elements. May be redefined by custom types that use random number generation.
  typedef T RandomDistributionElement;

  /// Type of the uniform distribution used in the generated expected by the random method.
  typedef typename DefaultRandomDistribution<RandomDistributionElement>::UniformDistribution UniformDistribution;

  /// Generate a random value using the provided generator. Needs to be defined for custom types that make use of DataSet::set_random_values.
  template <typename RandomGenerator>
  static T random(RandomGenerator &generator);
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
  static void deserialize(T &element, std::istream &in, Endianness::Type endianness = Endianness::host_endianness());
  static void deserialize_array(T *array, unsigned int size, std::istream &in, Endianness::Type endianness = Endianness::host_endianness());
};


/// Provides serialization operations for types that don't have a trivial serialization.
template <typename T>
struct SerializationTraits<T, false> {
  static void serialize(const T &element, std::ostream &out);
  static void serialize_array(const T *array, unsigned int size, std::ostream &out);
  static void deserialize(T &element, std::istream &in, Endianness::Type endianness = Endianness::host_endianness());
  static void deserialize_array(T *array, unsigned int size, std::istream &in, Endianness::Type endianness = Endianness::host_endianness());
};

// Serialization functions.
template <typename T> void serialize(const T &element, std::ostream &out);
template <typename T> void serialize_array(const T *array, unsigned int size, std::ostream &out);
template <typename T, const size_t Size> void serialize(const T (&array)[Size], std::ostream &out);

// Deserialization functions.
template <typename T> void deserialize(T &element, std::istream &in, Endianness::Type endianness = Endianness::host_endianness());
template <typename T> void deserialize_array(T *array, unsigned int size, std::istream &in, Endianness::Type endianness = Endianness::host_endianness());
template <typename T, const size_t Size> void deserialize(T (&array)[Size], std::istream &in, Endianness::Type endianness = Endianness::host_endianness());

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
struct Traits : NumericTraits<T>,
                DistanceNumericTraits<T>,
                RandomGenerationTraits<T>,
                CopyTraits<T>,
                EqualTraits<T>,
                SerializationTraits<T>,
                EndiannessTraits<T> {
  /// Type the traits are describing.
  typedef T Type;
};

} // namespace kche_tree

// Template implementation.
#include "traits.tpp"

#endif
