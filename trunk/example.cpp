/***************************************************************************
 *   Copyright (C) 2010 by Leandro Graciá Gil                              *
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
 * \file example.cpp
 * \brief Example program to illustrate the use of the kd_tree template.
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
//       kd-tree by just including the file "kche-tree_sse_24d.h" instead of "kche-tree.h".
//       Proper memory alignments are ensured by the specialization of the new operator.
//       Also remember to set the -msse flag when compiling with g++ or face errors.
//
#include "kche-tree.h"
using namespace kche_tree;

/// Number of dimensions to use in this test.
const unsigned int D = 24;

/// Alias for the specific kd_tree type being used.
typedef kd_tree<float, D> kdtree_test;

/**
 * Generate and initialize randomly a set of feature vectors.
 *
 * \param size Size of the set to generate.
 * \return A new set of \a size feature vectors initialized with random values in [-100, 100].
 */
kdtree_test::kd_point *generate_random_set(unsigned int size) {

  // Allocate the set.
  kdtree_test::kd_point *set = new kdtree_test::kd_point[size];

  // Initialize it randomly with values between -100 and 100.
  for (unsigned int i=0; i<size; ++i) {
    for (unsigned int d=0; d<D; ++d)
      set[i][d] = 100.0f * (rand() / (2.0f * RAND_MAX) - 1.0f);
  }

  // Return the new set.
  return set;
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
  const unsigned int N_train = 500000;
  kdtree_test::kd_point *train = generate_random_set(N_train);

  // Create and build a new kd-tree with the training set.
  kdtree_test kdtree;
  kdtree.build(train, N_train);

  // Generate 5 random feature vectors for testing.
  const unsigned int N_test = 5;
  kdtree_test::kd_point *test = generate_random_set(N_test);

  // Set the number of neighbours to retrieve and the epsilon used.
  const unsigned int K = 3;
  const float epsilon = 0.0f;

  // For each test case...
  for (unsigned int i=0; i<N_test; ++i) {

    // Retrieve the K nearest neighbours.
    std::vector<kdtree_test::kd_neighbour> neighbours;
    kdtree.knn(test[i], K, neighbours, epsilon);

    // Print distances to the K nearest neighbours.
    printf("Distance to nearest neighbours in test case %d: ", i + 1);
    for (unsigned int k=0; k<K; ++k)
      printf("%.4f ", sqrtf(neighbours[k].squared_distance));
    printf("\n");
  }

  // Release feature vector sets.
  delete []train;
  delete []test;

  return 0;
}
