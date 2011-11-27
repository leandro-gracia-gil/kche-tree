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
 * \file custom_type.h
 * \brief Custom type encapsulating a float value for testing the kche-tree library.
 * \author Leandro Graciá Gil
 */

// Include input and output streams.
#include <iostream>
#include <fstream>

// Include the kche-tree library.
#include "kche-tree/kche-tree.h"

/// Simple custom type embedding a floating point value.
class Custom {
public:
  /**
   * \brief Initialize to zero.
   *
   * In case our default constructor didn't do this, we should provide an alternate way by specializing kche_tree::Traits<T>::zero().
   */
  Custom() : value_(0.0f) {}

  // Constructor based on a floating point value.
  // Required only by the verification tool template, not to use kche-tree.
  Custom(float value) : value_(value) {}

  // Operators required by KDTree. Using default copy constructor and asignment operator.
  bool operator < (const Custom &a) const { return value_ < a.value_; }
  bool operator > (const Custom &a) const { return value_ > a.value_; }
  //bool operator <= (const Custom &a) const { return value_ <= a.value_; }
  bool operator == (const Custom &a) const { return value_ == a.value_; }

  // Extra operators required by the Euclidean metric and its incremental calculations.
  Custom& operator += (const Custom &a) { value_ += a.value_; return *this; }
  Custom& operator -= (const Custom &a) { value_ -= a.value_; return *this; }
  Custom& operator *= (const Custom &a) { value_ *= a.value_; return *this; }

  // Operators required for Mahalanobis metric matrix calculations.
  Custom& operator /= (const Custom &a) { value_ /= a.value_; return *this; }

  // Serialization methods: only required if we want to serialize the kd-tree or the data sets. Otherwise it can be safely ignored.
  /**
   * \brief Read from a stream.
   * Required instead of the >> operator since it doesn't provide the byte endianness of the data being read.
   *
   * \param in Input stream.
   * \param endianness Byte endianness of the input data.
   */
  Custom(std::istream &in, kche_tree::Endianness::Type endianness) { kche_tree::deserialize(value_, in, endianness); }
  friend std::ostream & operator << (std::ostream &out, const Custom &object); ///< Standard << stream operator.

  // Methods not required by the kche-tree library.
  float value() const { return value_; } ///< Get the encapsulated value.
  void set_value(float value) { value_ = value; } ///< Set the encapsulated value.

private:
  float value_; ///< Encapsulated value.
};

/// Write the embedded value into a stream. Used to print the value in the verification tool error messages.
std::ostream& operator << (std::ostream& out, const Custom &object) {
  out << object.value_;
  return out;
}

// Now provide some traits information to enable internal optimizations.
namespace kche_tree {

  /**
   * \brief Example trivial equality comparison trait for a custom type.
   *
   * C++ type traits don't provide a way to find out if a type implements the equality operator or not.
   * With this, we're telling kche-tree that our custom type can be compared by just raw memcmp and enable some optimizations.
   * Your type should satisfy is_pod<T> before doing this.
   */
  template <>
  struct HasTrivialEqual<Custom> {
    static const bool value = true;
  };

  /**
   * \brief Example trivial serialization trait for a custom type.
   *
   * C++ type traits don't provide either a way to find out if a type implements the stream operators or not.
   * With this, we're telling kche-tree that our custom type can be serialized by just reading/writing its memory and enable some optimizations.
   * Your type should satisfy is_pod<T> before doing this.
   */
  template <>
  struct HasTrivialSerialization<Custom> {
    static const bool value = true;
  };

  /**
   * \brief Example endianness traits for a custom type.
   *
   * Since we're implementing object serialization we need to provide a way to convert from little/big endian.
   * This trait provides a way to swap the endianness of the elements in the object.
   */
  template <>
  struct EndiannessTraits<Custom> {
    static void swap_endianness(Custom &value) {
      float aux = value.value();
      EndiannessTraits<float>::swap_endianness(aux);
      value.set_value(aux);
    }
  };

  /**
   * \brief Example numeric traits for a custom type.
   *
   * Because of the nature of the kd-tree structure many scalar functionalities are expected from the type used.
   * This trait defines these, being some of them used only in the Mahalanobis metric or by the verification tool.
   */
  template <>
  struct NumericTraits<Custom> {
    static Custom max() { return Custom(Traits<float>::max()); } ///< Maximum finite representable value.
    static Custom zero() { return Custom(); } ///< Equivalent to the additive identity (zero).
    static Custom one() { return Custom(1.0f); } ///< Equivalent to the multiplicative identity (one).
    static void negate(Custom &value) { value.set_value(-value.value()); } ///< Replace the value by its additive inverse (negative value).
    static void invert(Custom &value) { value.set_value(1.0f / value.value()); } ///< Replace the value by its multiplicative inverse (inverse value).

    typedef Custom AbsoluteValueType; ///< The absolute value of a Custom object is another custom object.
    static Custom abs(const Custom &value) { return Custom(fabs(value.value())); } ///< Return the absolute value.

    template <typename RandomGeneratorType>
    static Custom random(RandomGeneratorType &generator) { return Custom(generator()); } ///< Generate a random element using the provided generator.

    typedef float ExpectedDistributionElementType; ///< Random floats are used to generate Custom object's contents.
  };
}
