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
 * \file speed_kdtree.cpp
 * \brief Benchmark tool for kd-trees.
 * \author Leandro Graciá Gil
 */

// Includes from C Standard Library.
#include <cstdio>
#include <cstdlib>
#include <ctime>

// Include the generic kche-tree templates or the SSE-enabled specialization for floats and 24 dimensions.
#ifdef SSE
#include "kche-tree_sse_24d.h"
#else
#include "kche-tree.h"
#endif

using namespace kche_tree;

/// Number of dimensions to use in this test.
const unsigned int D = 24;

/// Alias for the specific kd_tree type being used.
typedef kd_tree<float, D> test_kdtree;


// Bring STL vectors from the STL namespace.
using std::vector;

#ifdef FROM_FILE
/**
 * Load a set of feature vectors from an input binary file.
 *
 * \param filename Name of the file containing the feature vectors.
 * \param dimensions Required number of dimensions of the vectors.
 * \param num_vectors Reference to an integer where the number of feature vectors will be set (output).
 * \return An array of contiguous feature vectors if successfully loaded, \c NULL otherwise.
 */
test_kdtree::kd_point *load_feature_vectors(const char *filename, unsigned int dimensions, unsigned int &num_vectors) {

  // Try to open the file.
  FILE *input = fopen(filename, "rb");
  if (input == NULL) {
    fprintf(stderr, "Error opening file '%s'\n", filename);
    return NULL;
  }

  // Read the number of vectors.
  if (fread(&num_vectors, sizeof(unsigned int), 1, input) != 1) {
    fprintf(stderr, "Error reading number of vectors from file '%s'\n", filename);
    fclose(input);
    return NULL;
  }

  // Check number of vectors.
  if (num_vectors == 0) {
    fprintf(stderr, "Invalid number of vectors in file '%s'\n", filename);
    fclose(input);
    return NULL;
  }

  // Read the number of dimensions of each vector.
  unsigned int file_dimensions;
  if (fread(&file_dimensions, sizeof(unsigned int), 1, input) != 1) {
    fprintf(stderr, "Error reading number of dimensions from file '%s'\n", filename);
    fclose(input);
    return NULL;
  }

  // Check number of dimensions.
  if (file_dimensions != dimensions) {
    fprintf(stderr, "Non-compatible number of dimensions in data (found %d, expected %d)\n", file_dimensions, dimensions);
    fclose(input);
    return NULL;
  }

  // Allocate memory for the feature vectors.
  test_kdtree::kd_point *vectors = new test_kdtree::kd_point[num_vectors];
  if (vectors == NULL) {
    fprintf(stderr, "Error allocating memory for %d feature vectors\n", num_vectors);
    fclose(input);
    return NULL;
  }

  // Read feature vector data.
  if (fread(vectors, sizeof(test_kdtree::kd_point), num_vectors, input) != num_vectors) {
    fprintf(stderr, "Error reading feature vector data from file '%s'\n", filename);
    delete [] vectors;
    fclose(input);
    return NULL;
  }

  // Close input file.
  fclose(input);

  // Return feature vectors.
  return vectors;
}
#endif

/**
 * Application entry point.
 * Two working modes are set by macros: generate random feature vectors (by default) or read them from a file.
 */
int main(int argc, char *argv[]) {

  #ifdef FROM_FILE
  // Set default parameters.
  int K;
  unsigned int N_train, N_test;
  float epsilon = 0.0f;
  bool ignore_point_in_tree = false;

  // Declare arrays of feature vectors.
  test_kdtree::kd_point *train = NULL, *test = NULL;

  // Read params.
  switch (argc) {
  case 6:
    if (!strcmp(argv[5], "true")) {
      ignore_point_in_tree = true;
    } else if (!strcmp(argv[5], "false")) {
      ignore_point_in_tree = false;
    } else {
      fprintf(stderr, "Invalid argument: '%s' should be true or false.\n", argv[5]);
      return 1;
    }

  case 5:
    // Set the epsilon value (search tolerance, 0 = deterministic).
    epsilon = atof(argv[4]);

  case 4:
    // Set pointers to training and testing sets.
    if ((train = load_feature_vectors(argv[2], D, N_train)) == NULL)
      return 1;
    if ((test  = load_feature_vectors(argv[3], D, N_test)) == NULL)
      return 1;

    // Set the number of neighbours to look for.
    K = atoi(argv[1]);
    break;

  default:
    // Wrong number of parameters.
    fprintf(stderr, "Usage: %s K train_file test_file [epsilon = 0.0] [ignore_point_in_tree = false]\n", argv[0]);
    return 1;
  }

  // Check params.
  if (N_train <= 0 || N_test <= 0 || K <= 0) {
    fprintf(stderr, "Error: invalid params\n");
    return 1;
  }

  #else
  // Set default parameters.
  int K, N_train, N_test;
  float range = 100.0f;
  int random_seed = -1;
  float epsilon = 0.0f;
  bool ignore_point_in_tree = false;

  // Read params.
  switch (argc) {
  case 8:
    // Set the random seed used to generate data (time is used if not provided).
    random_seed = atoi(argv[7]);

  case 7:
    if (!strcmp(argv[6], "true")) {
      ignore_point_in_tree = true;
    } else if (!strcmp(argv[6], "false")) {
      ignore_point_in_tree = false;
    } else {
      fprintf(stderr, "Invalid argument: '%s' should be true or false.\n", argv[6]);
      return 1;
    }

  case 6:
    // Set the range of the random feature vectors (from 0 to range).
    range = atof(argv[5]);

  case 5:
    // Set the epsilon value (search tolerance, 0 = deterministic).
    epsilon = atof(argv[4]);

  case 4:
    // Set the training and test set sizes.
    N_test  = atoi(argv[3]);
    N_train = atoi(argv[2]);

    // Set the number of neighbours to look for.
    K = atoi(argv[1]);

    break;

  default:
    // Wrong number of parameters.
    fprintf(stderr, "Usage: %s K N_train N_test [epsilon = 0.0] [range = 100.0] [ignore_point_in_tree = false] [random_seed]\n", argv[0]);
    return 1;
  }

  // Check params.
  if (N_train <= 0 || N_test <= 0 || K <= 0 || range == 0.0f) {
    fprintf(stderr, "Error: invalid params\n");
    return 1;
  }

  // Initialize random seed.
  if (random_seed == -1)
    srand(time(NULL));
  else
    srand(random_seed);

  // Build training random samples.
  test_kdtree::kd_point *train = new test_kdtree::kd_point[N_train];
  for (int i=0; i<N_train; ++i) {
    for (unsigned int d=0; d<D; ++d)
      train[i][d] = (rand() / (float) RAND_MAX) * range;
  }

  // Build testing random samples.
  test_kdtree::kd_point *test = new test_kdtree::kd_point[N_test];
  for (int i=0; i<N_test; ++i) {
    for (unsigned int d=0; d<D; ++d)
      test[i][d] = (rand() / (float) RAND_MAX) * range;
  }
  #endif

  // Build the kd-tree.
  clock_t t1_build = clock();
  test_kdtree kdtree;
  kdtree.build(train, N_train);
  clock_t t2_build = clock();

  // Process each test case.
  clock_t t1_test = clock();
  for (int i=0; i < (int) N_test; ++i) {

    // Get the K nearest neighbours.
    vector<test_kdtree::kd_neighbour> knn;
    kdtree.knn(test[i], K, knn, epsilon, ignore_point_in_tree);
  }
  clock_t t2_test = clock();

  // Release random samples.
  delete []train;
  delete []test;

  // Calculate times.
  double time_build = (t2_build - t1_build) / (double) CLOCKS_PER_SEC;
  double time_test  = (t2_test  - t1_test ) / (double) CLOCKS_PER_SEC;

  // Report results.
  printf("Kd-tree testing with D = %d, %d neighbours, epsilon %.2f, training set size %d, test set size %d\n", D, K, epsilon, N_train, N_test);
  printf("Build time: %.3f sec (%.2f%%)\n", time_build, 100.0 * time_build / (time_build + time_test));
  printf("Test  time: %.3f sec (%.2f%%) -- average per test: %.4f sec\n", time_test , 100.0 * time_test  / (time_build + time_test),
      time_test / (double) N_test);

  printf("Total time: %.3f sec\n", time_build + time_test);

  return 0;
}
