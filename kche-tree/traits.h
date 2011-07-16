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
#include <tr1/type_traits>
#else
#include <cstdint>
#include <type_traits>
#endif

// Demangling functions for the GNU C++ compiler.
#ifdef __GNUG__
#include <cxxabi.h>
#endif

// Include STL exceptions.
#include <stdexcept>

// Include compile-time assertions, endianness operations, smart pointers and optimized params.
#include "compile_assert.h"
#include "endianness.h"
#include "smart_ptr.h"
#include "rparam.h"

namespace kche_tree {

/**
 * \brief Define if a type is a fundamental one. This is basically a shortcut to the equivalent STL or TR1 type trait.
 */
template <typename T>
struct IsFundamental :
#ifdef KCHE_TREE_DISABLE_CPP0X
    std::tr1::integral_constant<bool, std::tr1::is_fundamental<T>::value>
#else
    std::integral_constant<bool, std::is_fundamental<T>::value>
#endif
  {};

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
struct has_trivial_equal : IsFundamental<T> {};

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
struct has_trivial_serialization : IsFundamental<T> {};

// Forward-declare the templates provided here.
template <typename T> struct Traits;
template <typename T> struct AccumulationTraits;
template <typename T, bool hasTrivialEqual = has_trivial_equal<T>::value> struct EqualTraits;
template <typename T, bool hasTrivialSerialization = has_trivial_serialization<T>::value> struct SerializationTraits;

template <typename T, bool hasTrivialCopy =
#ifdef KCHE_TREE_DISABLE_CPP0X
  std::tr1::has_trivial_copy<T>::value
#else
  std::has_trivial_copy_constructor<T>::value
#endif
  > struct CopyTraits;

template <typename T, bool IsFundamental = IsFundamental<T>::value>
struct EndiannessTraits;

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
   * \brief Compare the contents of two arrays of the given size.
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
   * \brief Compare the contents of two arrays of the given size.
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
   * \brief Copy the contents of one array to another.
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
   * \brief Copy the contents of one array to another.
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
 * \brief Provides serialization operations for types that have a trivial serialization.
 */
template <typename T>
struct SerializationTraits<T, true> {
  /**
   * \brief Serialize an element into the provided stream.
   * This version is optimized to write raw memory values.
   *
   * \note Data is serialized with the same endianness as the local host.
   * \param element Element to be written.
   * \param out Output stream.
   */
  static void serialize(const T &element, std::ostream &out) {
    out.write(reinterpret_cast<const char *>(&element), sizeof(T));
  }

  /**
   * \brief Serialize an array into the provided stream.
   * This version is optimized to write raw memory values.
   *
   * \note Data is serialized with the same endianness as the local host.
   * \param array Array to be written.
   * \param size Number of elements in \a array.
   * \param out Output stream.
   */
  static void serialize_array(const T *array, unsigned int size, std::ostream &out) {
    out.write(reinterpret_cast<const char *>(array), size * sizeof(T));
  }

  /**
   * \brief Deserialize an element from the provided stream.
   * This version is optimized to read raw memory values.
   * Endianness is corrected to match host's if required.
   *
   * \param element Element to be read.
   * \param in Input stream.
   * \param endianness Endianness of the serialized data. Defaults to host's endianness.
   */
  static void deserialize(T &element, std::istream &in, Endianness::Type endianness = Endianness::endianness()) {
    in.read(reinterpret_cast<char *>(&element), sizeof(T));
    if (sizeof(T) > 1 && endianness != Endianness::endianness())
      Traits<T>::swap_endianness(element);
  }

  /**
   * \brief Deserialize an array from the provided stream.
   * This version is optimized to read raw memory values.
   * Endianness is corrected to match host's if required.
   *
   * \param array Array to be read.
   * \param size Number of elements in \a array.
   * \param in Input stream.
   * \param endianness Endianness of the serialized data. Defaults to host's endianness.
   */
  static void deserialize_array(T *array, unsigned int size, std::istream &in, Endianness::Type endianness = Endianness::endianness()) {
    in.read(reinterpret_cast<char *>(array), size * sizeof(T));
    if (sizeof(T) > 1 && endianness != Endianness::endianness())
      for (unsigned int i=0; i<size; ++i)
        Traits<T>::swap_endianness(array[i]);
  }
};

/**
 * \brief Provides serialization operations for types that don't have a trivial serialization.
 */
template <typename T>
struct SerializationTraits<T, false> {
  /**
   * \brief Serialize an element into the provided stream.
   * This version makes use of the << operator.
   *
   * \note Data is serialized with the same endianness as the local host.
   * \param element Element to be written.
   * \param out Output stream.
   */
  static void serialize(const T &element, std::ostream &out) {
    out << element;
  }

  /**
   * \brief Serialize an array into the provided stream.
   * This version makes use of the << operator.
   *
   * \note Data is serialized with the same endianness as the local host.
   * \param array Array to be written.
   * \param size Number of elements in \a array.
   * \param out Output stream.
   */
  static void serialize_array(const T *array, unsigned int size, std::ostream &out) {
    for (unsigned int i=0; i<size; ++i)
      serialize(array[i], out);
  }

  /**
   * \brief Deserialize an element from the provided stream.
   * Endianness is corrected to match host's if required.
   * This version makes use of the >> operator.
   *
   * \param element Element to be read.
   * \param in Input stream.
   * \param endianness Endianness of the serialized data. Defaults to host's endianness.
   */
  static void deserialize(T &element, std::istream &in, Endianness::Type endianness = Endianness::endianness()) {
    element = T(in, endianness);
  }

  /**
   * \brief Deserialize an array from the provided stream.
   * Endianness is corrected to match host's if required.
   * This version makes use of the >> operator.
   *
   * \param array Array to be read.
   * \param size Number of elements in \a array.
   * \param in Input stream.
   * \param endianness Endianness of the serialized data. Defaults to host's endianness.
   */
  static void deserialize_array(T *array, unsigned int size, std::istream &in, Endianness::Type endianness = Endianness::endianness()) {
    for (unsigned int i=0; i<size; ++i)
      deserialize(array[i], in, endianness);
  }
};

/**
 * \brief Convenience function to serialize an element.
 * Will make use of optimizations if possible.
 *
 * \note Data is serialized with the same endianness as the local host.
 * \param element Element to be written.
 * \param out Output stream.
 */
template <typename T>
void serialize(const T &element, std::ostream &out) {
  SerializationTraits<T>::serialize(element, out);
}

/**
 * \brief Convenience function to serialize an array type.
 *
 * \note Data is serialized with the same endianness as the local host.
 * \param array Array to be written.
 * \param out Output stream.
 */
template <typename T, const size_t Size>
void serialize(const T (&array)[Size], std::ostream &out) {
  SerializationTraits<T>::serialize_array(array, Size, out);
}

/**
 * \brief Convenience function to serialize an element.
 * Will make use of optimizations if possible.
 *
 * \note Data is serialized with the same endianness as the local host.
 * \param array Array to be written.
 * \param size Number of elements in \a array.
 * \param out Output stream.
 */
template <typename T>
void serialize_array(const T *array, unsigned int size, std::ostream &out) {
  SerializationTraits<T>::serialize_array(array, size, out);
}

/**
 * \brief Convenience function to deserialize an element.
 * Endianness is corrected to match host's if required.
 * Will make use of optimizations if possible.
 *
 * \param element Element to be read.
 * \param in Input stream.
 * \param endianness Endianness of the serialized data. Defaults to host's endianness.
 */
template <typename T>
void deserialize(T &element, std::istream &in, Endianness::Type endianness = Endianness::endianness()) {
  SerializationTraits<T>::deserialize(element, in, endianness);
}

/**
 * \brief Convenience function to deserialize an array type.
 * Endianness is corrected to match host's if required.
 *
 * \param array Array to be read.
 * \param in Input stream.
 * \param endianness Endianness of the serialized data. Defaults to host's endianness.
 */
template <typename T, const size_t Size>
void deserialize(T (&array)[Size], std::istream &in, Endianness::Type endianness = Endianness::endianness()) {
  SerializationTraits<T>::deserialize_array(array, Size, in, endianness);
}

/**
 * \brief Convenience function to deserialize an array.
 * Endianness is corrected to match host's if required.
 * Will make use of optimizations if possible.
 *
 * \param array Array to be read.
 * \param size Number of elements in \a array.
 * \param in Input stream.
 * \param endianness Endianness of the serialized data. Defaults to host's endianness.
 */
template <typename T>
void deserialize_array(T *array, unsigned int size, std::istream &in, Endianness::Type endianness = Endianness::endianness()) {
  SerializationTraits<T>::deserialize_array(array, size, in, endianness);
}

/**
 * \brief Provide mechanisms for type serialization and checking.
 */
template <typename T>
struct TypeSerializationTraits {
  /**
   * \brief Serialize the information identifying a type into an output stream.
   *
   * Writes the name (demangled if possible) of the type into the stream.
   * \note Data is serialized with the same endianness as the local host.
   * \param out Output stream.
   */
  static void serialize_type(std::ostream &out) {
    uint32_t name_length = strlen(Traits<T>::name());
    serialize(name_length, out);
    serialize_array(Traits<T>::name(), name_length, out);
  }

  /**
   * \brief Deserialize the type information from an input stream.
   *
   * Checks the type name and throws an exception if it doesn't match.
   * \param in Input stream.
   * \param endianness Endianness of the serialized data. Defaults to host's endianness.
   * \exception std::runtime_error Thrown when the serialized type doesn't match \a T.
   */
  static void check_serialized_type(std::istream &in, Endianness::Type endianness = Endianness::endianness()) {

    // Read type name length.
    uint32_t name_length;
    deserialize(name_length, in, endianness);
    if (!in.good())
      throw std::runtime_error("error reading type name length data");

    // Read type name.
    ScopedArray<char> type_name(new char[name_length + 1]);
    deserialize_array(type_name.get(), name_length, in);
    if (!in.good())
      throw std::runtime_error("error reading type name");
    type_name[name_length] = '\0';

    // Check type name.
    // WARNING: The value returned by Traits<T>::name() is the demangled (if supported) version of typeid::name()
    //          and hence is implementation-dependent.
    //          There is a possibility of incompatible files when porting data between different platforms.
    if (strcmp(Traits<T>::name(), type_name.get())) {
      std::string error_msg = "type doesn't match: found ";
      error_msg += type_name.get();
      error_msg += ", expected ";
      error_msg += Traits<T>::name();
      throw std::runtime_error(error_msg);
    }
  }
};

/**
 * \brief Provides functionality related to the name of the type being described.
 */
template <typename T>
struct NameTraits {
  /**
   * \brief Provide if possible the demangled name of the type \a T.
   *
   * The returned value is highly dependent on the platform and the compiler.
   * This method intends to return a full C++ style readable version of the type.
   * It should be extended in the future to include support for extra compilers and platforms.
   */
  static const char *name() {
    static const char *typeid_name = typeid(T).name();
    #ifdef __GNUG__
    static int status;
    static const char *realname = abi::__cxa_demangle(typeid_name, 0, 0, &status);
    if (!status)
      return realname;
    #endif
    return typeid_name;
  }
};

/**
 * \brief Provides endianness swapping for fundamental types.
 */
template <typename T>
struct EndiannessTraits<T, true> {
  static void swap_endianness(T &value) {
    EndiannessSwapper<T>::run(reinterpret_cast<uint8_t *>(&value));
  }
};

/**
 * \brief Provides endianness swapping for non-fundamental types.
 * Custom types should add their specializations here.
 */
template <typename T>
struct EndiannessTraits<T, false> {};

/**
 * \brief Provides endianness swapping for array types.
 *
 * \param array Array to swap endianness.
 */
template <typename T, const size_t Size>
void swap_endianness(T (&array)[Size]) {
  for (unsigned int i=0; i<Size; ++i)
    EndiannessTraits<T>::swap_endianness(array[i]);
}

/**
 * \brief Provides specific information about the type \a T.
 *
 * \tparam T Type which information is being provided.
 */
template <typename T>
struct Traits : TraitsBase<T>,
                AccumulationTraits<T>,
                NameTraits<T>,
                CopyTraits<T>,
                EqualTraits<T>,
                SerializationTraits<T>,
                TypeSerializationTraits<T>,
                EndiannessTraits<T> {};

// Bring to the kche_tree namespace the appropriate is_same template.
using
#ifdef KCHE_TREE_DISABLE_CPP0X
    std::tr1::is_same;
#else
    std::is_same;
#endif

} // namespace kche_tree

#endif
