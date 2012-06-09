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
  /// Initialize to zero.
  /// In case our default constructor didn't do this, we should provide an alternate way by specializing kche_tree::NumericTraits<Custom>::zero().
  Custom() : value_(0.0f) {}

  // Operators required by KDTree. Using default copy constructor and asignment operator.
  bool operator < (const Custom &a) const { return value_ < a.value_; }
  bool operator > (const Custom &a) const { return value_ > a.value_; }
  bool operator == (const Custom &a) const { return value_ == a.value_; }

  // Serialization methods: only required if we want to serialize the kd-tree or the data sets. Otherwise it can be safely ignored.
  // Read from a stream. Required like this instead of the >> operator since it doesn't provide the byte endianness of the data being read.
  Custom(std::istream &in, kche_tree::Endianness::Type endianness) { kche_tree::deserialize(value_, in, endianness); }

  // Standard << stream operator.
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
   * Defines the type and the method used when calculating distances between elements.
   * Additionally, the mean of a list of objects is provided in order to enable the automatic
   * calculation of the inverse covariance matrix for the Mahalanobis metric cases.
   */
  template <>
  struct NumericTraits<Custom> {
    typedef float Distance; ///< Distances between custom elements are encoded and calculated using floats.

    /// This is how calculate the direct distance between two Custom objects. RParam<Custom>::Type is just a const reference.
    static Distance distance(typename RParam<Custom>::Type a, typename RParam<Custom>::Type b) {
      return a.value() - b.value();
    }

    /// Equivalent to the zero value for Custom objects.
    static Custom zero() { return Custom(); }

    /// Creates a mean object from a list defined by the iterators. This is required in order to calculate
    /// the inverse covariance matrix of a data set as part of the Mahalanobis distance. It is safe to leave
    /// a dummy implementation if the method \link kche_tree::MahalanobisMetric<T, D>::set_inverse_covariance set_inverse_covariance \endlink
    /// for data sets is not used.
    template <typename BidirectionalElementIterator>
    static Custom mean(const BidirectionalElementIterator &begin, const BidirectionalElementIterator &end) {
      float acc = 0.0f;
      unsigned int num_elements = 0;
      for (BidirectionalElementIterator it = begin; it != end; ++it) {
        acc += it->value();
        ++num_elements;
      }

      Custom object;
      object.set_value(acc / num_elements);
      return object;
    }
  };

  /**
   * \brief Example random generation traits for a custom type.
   *
   * Specifies how new random Custom objects can be created by the library, and based in which random type.
   */
  template <>
  struct RandomGenerationTraits<Custom> {
    /// Defines the type of the objects returned by the random generator. In this case, random floats are used to build Custom objects.
    typedef float RandomDistributionElement;

    /// Type of the uniform distribution used to generate the input random floats.
    typedef UniformReal<RandomDistributionElement> UniformDistribution;

    /// Defines how to construct Custom objects from a random data generator.
    template <typename RandomGenerator>
    static Custom random(RandomGenerator &generator) { Custom object; object.set_value(generator()); return object; }
  };
}
