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
 * \file custom_type.cpp
 * \brief Example program to show how to use custom type objects with the Kche-tree library.
 * \author Leandro Graciá Gil
 */

// C and C++ Standard Library includes.
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>

// Include the kche-tree templates.
#include "kche-tree/kche-tree.h"

using namespace kche_tree;
using namespace std;

// ------------------------------------------------------- //
//            Custom type definition and traits.           //
// ------------------------------------------------------- //

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

  // Methods not required by the kche-tree library.
  float value() const { return value_; } ///< Get the encapsulated value.
  void set_value(float value) { value_ = value; } ///< Set the encapsulated value.

private:
  float value_; ///< Encapsulated value.
};

// Now provide some traits information to tell the library how to use our custom type.
namespace kche_tree {

  /// Example numeric traits for a custom type.
  /// Defines the type and the method used when calculating distances between elements.
  template <>
  struct NumericTraits<Custom> {
    /// Distances between custom elements are encoded and calculated using floats.
    typedef float Distance;

    /// This is how calculate the direct distance between two Custom objects. RParam<Custom>::Type is just a const reference.
    static Distance distance(typename RParam<Custom>::Type a, typename RParam<Custom>::Type b) {
      return a.value() - b.value();
    }

    /// Equivalent to the zero value for Custom objects.
    static Custom zero() { return Custom(); }

    /// Creates a mean object from a list defined by the iterators.
    /// This is required in order to calculate the inverse covariance matrix of a data set as part of the Mahalanobis distance.
    /// It is safe to leave a dummy implementation as the method kche_tree::MahalanobisMetric<T, D>::set_inverse_covariance for data sets is not used.
    template <typename BidirectionalElementIterator>
    static Custom mean(const BidirectionalElementIterator &begin, const BidirectionalElementIterator &end) {
      return Custom();
    }
  };

  /// Example random generation traits for a custom type.
  /// Specifies how new random Custom objects can be created by the library, and based in which random type.
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

// ------------------------------------------------------- //
// ------------------------------------------------------- //

// Data type and number of dimensions to use in this example.
typedef Custom Type;
const unsigned int Dimensions = 24;

// Alias for the specific KDTree and data set types being used.
typedef KDTree<Type, Dimensions> KDTreeTest;
typedef DataSet<Type, Dimensions> DataSetTest;

int main(int argc, char *argv[]) {

  // Initialize the random seed.
  DefaultRandomEngine random_engine;
  random_engine.seed(time(NULL));

  // Create a uniform distribution between -100 and 100.
  typedef Traits<Type>::UniformDistribution UniformDistribution;
  UniformDistribution value_distribution(-100.0f, 100.0f);

  // Bind them together into a random number generator. Use operator () to get random numbers.
  RandomGenerator<DefaultRandomEngine, UniformDistribution> generator(random_engine, value_distribution);

  // Generate 500000 random feature vectors for training.
  DataSetTest train_set(500000);
  train_set.set_random_values(generator);

  // Create and build a new kd-tree with the training set.
  KDTreeTest kdtree;
  kdtree.build(train_set);

  // Generate 5 random feature vectors for testing.
  DataSetTest test_set(5);
  test_set.set_random_values(generator);

  // Set the number of neighbours to retrieve.
  const unsigned int K = 3;

  // For each test case...
  for (unsigned int i=0; i<test_set.size(); ++i) {
    // Retrieve the K nearest neighbors. KNeighbours is just a vector of Neighbor objects, which are
    // basically tuples of index and squared distances.
    KDTreeTest::KNeighbors neighbors;

    #ifdef KCHE_TREE_DISABLE_CPP1X
    // C++1x allows the use of default template method arguments, which lets us to automatically use
    // K-Vectors (vectors holding the K closer values) and the Euclidean metric unless specified.
    // Unfortunately, without C++1x explicit values are required.
    kdtree.knn<KVector>(test_set[i], K, neighbors, EuclideanMetric<Type, Dimensions>());
    #else
    kdtree.knn(test_set[i], K, neighbors);
    #endif

    // Print distances to the K nearest neighbours.
    cout << "Distance to the " << K << " nearest neighbours in test case " << (i + 1) << ": ";
    cout << fixed;
    for (unsigned int k=0; k<K; ++k)
      cout << setprecision(4) << sqrtf(neighbors[k].squared_distance()) << " ";
    cout << endl;
  }

  return 0;
}
