/***************************************************************************
 *   Copyright (C) 2010, 2011 by Leandro Graciá Gil                        *
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
 * \file knn_simple.cpp
 * \brief Example program to show how to perform a simple K-Nearest Neighbour search using kche-tree.
 * \author Leandro Graciá Gil
 */

// C Standard Library includes.
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>

// Include the kche-tree templates.
//
// Note: in the float and 24 dimensions case we can use an SSE-accelerated version of the
//       kd-tree by just including the file "../tools/kche-tree_sse_24d.h" instead of "kche-tree.h".
//       Proper memory alignments are ensured by the specialization of the new operator.
//       Also remember to set the -msse flag when compiling with g++ or face errors.
//
#include "kche-tree/kche-tree.h"
using namespace kche_tree;

/// Number of dimensions to use in this example.
const unsigned int D = 24;

/// Alias for the specific KDTree and data set types being used.
typedef KDTree<float, D> KDTreeTest;
typedef DataSet<float, D> DataSetTest;

/**
 * Generate and initialize randomly a set of feature vectors.
 *
 * \param dataset Data set to fill with random values.
 */
void generate_random_set(DataSetTest &dataset) {

  // Initialize it randomly with values between -100 and 100.
  for (unsigned int i=0; i<dataset.size(); ++i) {
    for (unsigned int d=0; d<D; ++d)
      dataset[i][d] = 100.0f * (rand() / (2.0f * RAND_MAX) - 1.0f);
  }
}

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
  generate_random_set(train_set);

  // Create and build a new kd-tree with the training set.
  KDTreeTest kdtree;
  kdtree.build(train_set);

  // Generate 5 random feature vectors for testing.
  DataSetTest test_set(5);
  generate_random_set(test_set);

  // Set the number of neighbours to retrieve.
  const unsigned int K = 3;

  // For each test case...
  for (unsigned int i=0; i<test_set.size(); ++i) {

    // Retrieve the K nearest neighbours.
    std::vector<KDTreeTest::NeighbourType> neighbours;
    #ifdef KCHE_TREE_DISABLE_CPP0X
    // The Euclidean metric is used by default if C++0x is enabled, but explicitely required if disabled.
    kdtree.knn<KVector>(test_set[i], K, neighbours, EuclideanMetric<float, D>());
    #else
    kdtree.knn(test_set[i], K, neighbours);
    #endif

    // Print distances to the K nearest neighbours.
    printf("Distance to the %d nearest neighbours in test case %d: ", K, i + 1);
    for (unsigned int k=0; k<K; ++k)
      printf("%.4f ", sqrtf(neighbours[k].squared_distance));
    printf("\n");
  }

  return 0;
}
