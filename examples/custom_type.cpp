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
 * \file custom_type.cpp
 * \brief Example program to show how to use custom type objects with the Kche-tree library.
 * \author Leandro Graciá Gil
 */

// C Standard Library includes.
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>

// Include the kche-tree templates.
#include "kche-tree/kche-tree.h"
using namespace kche_tree;

/// Our very simple custom type. Encapsulates a floating point value.
class Custom {
public:
  /**
   * Initialize to zero.
   *
   * In case our default constructor didn't do this, we should provide a way to initialize to zero by specializing kche_tree::Traits<T>::zero().
   */
  Custom() : value_(0.0f) {}

  // Operators required by KDTree. Using default copy constructor and asignment operator.
  bool operator < (const Custom &a) const { return value_ < a.value_; }
  bool operator > (const Custom &a) const { return value_ > a.value_; }
  bool operator <= (const Custom &a) const { return value_ <= a.value_; }
  bool operator >= (const Custom &a) const { return value_ >= a.value_; } // Required only if Settings::verify_kdtree_after_deserializing is enabled;
  bool operator == (const Custom &a) const { return value_ == a.value_; }

  // Extra operators required by the Euclidean metric and its incremental calculations.
  Custom& operator += (const Custom &a) { value_ += a.value_; return *this; }
  Custom& operator -= (const Custom &a) { value_ -= a.value_; return *this; }
  Custom& operator *= (const Custom &a) { value_ *= a.value_; return *this; }

  // Serialization methods: only required if we want to serialize the kd-tree or the data sets. Otherwise it can be safely ignored.
  // Read from a stream. Required like this instead of the >> operator since it doesn't provide the byte endianness of the data being read.
  Custom(std::istream &in, Endianness::Type endianness) { deserialize(value_, in, endianness); }

  // Standard << stream operator.
  friend std::ostream & operator << (std::ostream &out, const Custom &object);

  // Non kche-tree methods:
  /// Get the encapsulated value.
  float value() const { return value_; }

  /// Initialize to a random value between -100 and 100.
  void set_random() {
    value_ = 100.0f * (rand() / (2.0f * RAND_MAX) - 1.0f);
  }

private:
  float value_;
};

// Write the contents into a stream. Will use the local endianness.
std::ostream& operator << (std::ostream& out, const Custom &object) {
  serialize(object.value_, out);
  return out;
}

// Now provide some traits information to enable internal optimizations.
namespace kche_tree {

  // C++ type traits don't provide a way to find out if a type implements the equality operator or not.
  // With this, we're telling kche-tree that our custom type can be compared by just raw memcmp and enable some optimizations.
  // Your type should satisfy std::tr1::is_pod<T> before doing this.
  template <>
  struct has_trivial_equal<Custom> {
    static const bool value = true;
  };

  // C++ type traits don't provide either a way to find out if a type implements the stream operators or not.
  // With this, we're telling kche-tree that our custom type can be serialized by just reading/writing its memory and enable some optimizations.
  // Your type should satisfy std::tr1::is_pod<T> before doing this.
  template <>
  struct has_trivial_serialization<Custom> {
    static const bool value = true;
  };
}

/// Number of dimensions to use in this example.
const unsigned int D = 24;

/// Alias for the specific KDTree and data set types being used.
typedef KDTree<Custom, D> KDTreeTest;
typedef DataSet<Custom, D> DataSetTest;

/**
 * Tool entry point.
 */
int main(int argc, char *argv[]) {

  // Note: this tool has been designed for illustrative purposes only.
  //       For result testing please use test_kdtree and its variations.
  //       For benchmarking use speed_kdtree and its variations.

  // Initialize the random seed.
  srand(time(NULL));

  // Generate 500000 random feature vectors for training.
  DataSetTest train_set(500000);
  for(unsigned int i=0; i<train_set.size(); ++i)
    for(unsigned int d=0; d<D; ++d)
      train_set[i][d].set_random();

  // Create and build a new kd-tree with the training set.
  KDTreeTest kdtree;
  kdtree.build(train_set);

  // Generate 5 random feature vectors for testing.
  DataSetTest test_set(5);
  for(unsigned int i=0; i<test_set.size(); ++i)
    for(unsigned int d=0; d<D; ++d)
      test_set[i][d].set_random();

  // Set the number of neighbours to retrieve.
  const unsigned int K = 3;

  // For each test case...
  for (unsigned int i=0; i<test_set.size(); ++i) {

    // Retrieve the K nearest neighbours.
    std::vector<KDTreeTest::NeighbourType> neighbours;
    #ifdef KCHE_TREE_DISABLE_CPP0X
    // The Euclidean metric is used by default if C++0x is enabled, but explicitely required if disabled.
    kdtree.knn<KVector>(test_set[i], K, neighbours, EuclideanMetric<Custom, D>());
    #else
    kdtree.knn(test_set[i], K, neighbours);
    #endif

    // Print distances to the K nearest neighbours.
    printf("Distance to the %d nearest neighbours in test case %d: ", K, i + 1);
    for (unsigned int k=0; k<K; ++k)
      printf("%.4f ", sqrtf(neighbours[k].squared_distance.value()));
    printf("\n");
  }

  return 0;
}
