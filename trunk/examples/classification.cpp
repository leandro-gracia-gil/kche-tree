/***************************************************************************
 *   Copyright (C) 2012 by Leandro Graciá Gil                              *
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
 * \file classification.cpp
 * \brief Example program to show a simple vector classification using knn.
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

// Data type and number of dimensions to use in this example.
typedef float Type;
const unsigned int Dimensions = 24;
typedef int Label;

// Number of classes to use in this example.
const unsigned int Classes = 3;

// Alias for the specific KDTree and data set types being used.
typedef KDTree<Type, Dimensions> KDTreeTest;

// ------------------------------------------------------- //
//      Note that this time the data set is labeled.       //
// ------------------------------------------------------- //
typedef LabeledDataSet<Type, Dimensions, Label> DataSetTest;
// ------------------------------------------------------- //
// ------------------------------------------------------- //

void generate_random_labels(DefaultRandomEngine &random_engine, DataSetTest &dataset) {

  // Generate random integer labels for the different classes.
  typedef Traits<Label>::UniformDistribution UniformLabelDistribution;
  UniformLabelDistribution label_distribution(0, Classes - 1);
  RandomGenerator<DefaultRandomEngine, UniformLabelDistribution> label_generator(random_engine, label_distribution);

  // Assign the labels to the vectors in the data set.
  for (unsigned int i=0; i<dataset.size(); ++i)
    dataset.label(i) = label_generator();
}

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

  // Set random labels to the vectors.
  generate_random_labels(random_engine, train_set);

  // Create and build a new kd-tree with the training set.
  KDTreeTest kdtree;
  kdtree.build(train_set);

  // Generate 5 random feature vectors for testing.
  DataSetTest test_set(5);
  test_set.set_random_values(generator);

  // Set the number of neighbours to retrieve.
  const unsigned int K = 5;

  // For each test case...
  for (unsigned int i=0; i<test_set.size(); ++i) {
    // Retrieve the K nearest neighbors. KNeighbours is just a vector of Neighbor objects, which are
    // basically tuples of index and squared distances.
    KDTreeTest::KNeighbors neighbors;

    #ifdef KCHE_TREE_DISABLE_CPP1X
    // C++1x allows the use of default template method arguments, which lets us to automatically use
    // K-Vectors (vectors holding the K closer values) and the Euclidean metric unless specified.
    // Unfortunately, without C++1x explicit values are required.
    kdtree.knn<KVector>(test_set[i], K, neighbors, mahalanobis_metric);
    #else
    kdtree.knn(test_set[i], K, neighbors, mahalanobis_metric);
    #endif

    // Get the most frequent class among neighbors.
    // TODO: use methods in LabeledDataSet.

    // Print distances to the K nearest neighbours.
    cout << "Distance to the " << K << " nearest neighbours in test case " << (i + 1) << ": ";
    cout << fixed;
    for (unsigned int k=0; k<K; ++k)
      cout << setprecision(4) << sqrtf(neighbors[k].squared_distance()) << " ";
    cout << endl;
  }

  return 0;
}
