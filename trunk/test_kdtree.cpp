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
 * \file test_kdtree.cpp
 * \brief Testing tool for kd-trees.
 * \author Leandro Graciá Gil
 */

// Includes from C Standard Library and C++ STL.
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cfloat>
#include <fstream>

// Enable kd-tree structure debugging.
#define DEBUG_KDTREE

// Include the generic kd-tree template or its SSE-enabled specialization for floats and 24 dimensions.
#ifdef SSE
#include "kd-tree_sse_24d.h"
#else
#include "kd-tree.h"
#endif

/// Number of dimensions to use in this test.
const unsigned int D = 24;

/// Alias for the specific kd_tree type being used.
typedef kd_tree<float, D> test_kdtree;


// Bring some things from STL namespace.
using std::ofstream;
using std::ifstream;
using std::vector;
using std::min;
using std::max;


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

  // Declare arrays of feature vectors.
  test_kdtree::kd_point *train = NULL, *test = NULL;

  // Read params.
  switch (argc) {
  case 5:
    // Set the epsilon value (search tolerance, 0 = deterministic).
    epsilon = atof(argv[4]);

  case 4:
    // Set pointers to training and testing sets.
    if ((train = load_feature_vectors(argv[2], D, N_train)) == NULL)
      return 1;
    if ((test = load_feature_vectors(argv[3], D, N_test)) == NULL)
      return 1;

    // Set the number of neighbours to look for.
    K = atoi(argv[1]);
    break;

  default:
    // Wrong number of parameters.
    fprintf(stderr, "Usage: %s K train_file test_file [epsilon = 0.0]\n", argv[0]);
    return 1;
  }

  // Check params.
  if (N_train <= 0 || N_test <= 0 || K <= 0) {
    fprintf(stderr, "Error: invalid params\n");
    return 1;
  }

  // Find the maximum and minimum points (used to estimate range searching values).
  test_kdtree::kd_point min_point = train[0];
  test_kdtree::kd_point max_point = train[0];
  for (unsigned int i=1; i<N_train; ++i) {
    for (unsigned int d=0; d<D; ++d) {
      min_point[d] = min(min_point[d], train[i][d]);
      max_point[d] = max(max_point[d], train[i][d]);
    }
  }

  // Find the points nearest to maximum and minimum points.
  unsigned int min_nearest = 0, max_nearest = 0;
  float min_distance = min_point.distance_to(train[min_nearest]);
  float max_distance = max_point.distance_to(train[max_nearest]);

  for (unsigned int i=1; i<N_train; ++i) {

    float dist = min_point.distance_to(train[i]);
    if (dist < min_distance) {
      min_nearest = i;
      min_distance = dist;
    }

    dist = max_point.distance_to(train[i]);
    if (dist < max_distance) {
      max_nearest = i;
      max_distance = dist;
    }
  }

  // Estimate range.
  float range = sqrt(train[min_nearest].distance_to(train[max_nearest])) / M_SQRT1_2;

  #else
  // Set default parameters.
  int K, N_train, N_test;
  int random_seed = -1;
  float range = 100.0f;
  float epsilon = 0.0f;

  // Read params.
  switch (argc) {
  case 7:
    // Set the random seed used to generate data (time is used if not provided).
    random_seed = atoi(argv[6]);

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
    fprintf(stderr, "Usage: %s K N_train N_test [epsilon = 0.0] [range = 100.0] [random_seed]\n", argv[0]);
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
    // Have a 20% probability of reusing training set samples.
    // This is used to test the 'point already in tree' functionality.
    if ((rand() % 100) < 20) {
      int train_index = rand() % N_train;
      memcpy(&test[i][0], &train[train_index][0], D * sizeof(float));
    } else {
      for (unsigned int d=0; d<D; ++d)
        test[i][d] = (rand() / (float) RAND_MAX) * range;
    }
  }
  #endif

  // Build the kd-tree.
  test_kdtree kdtree;
  kdtree.build(train, N_train);

  // Try to save the kd-tree to a file.
  ofstream out_file("kd-tree.test", std::ios::binary | std::ios::out);
  out_file << kdtree;
  out_file.close();

  // Read the tree back.
  ifstream in_file("kd-tree.test", std::ios::binary | std::ios::in);
  in_file >> kdtree;
  in_file.close();

  // Test the subscript operator.
  bool ok = true;
  for (int i=0; i < (int) N_train; ++i) {
    if (train[i] != kdtree[i]) {
      fprintf(stderr, "Non-matching subscript operator value for index %d (error in 1st axis: %.6g)\n", i, fabs(train[i][0] - kdtree[i][0]));
      ok = false;
    }
  }

  // Set the tolerance value used for checking results.
  const float tolerance = 1e-2;

  // Allocate memory for the exhaustive calculation of nearest neighbours.
  test_kdtree::kd_neighbour *nearest = new test_kdtree::kd_neighbour[N_train];

  // Process each test case.
  for (int i=0; i < (int) N_test; ++i) {

    #ifdef FROM_FILE
    bool points_in_tree = false;
    #else
    bool points_in_tree = rand() & 1;
    #endif

    // Create a vector of point-distance tuples to current test point.
    for (int n=0; n < (int) N_train; ++n) {
      float distance = 0.0f;
      for (unsigned int d=0; d<D; ++d)
        distance += (train[n][d] - test[i][d]) * (train[n][d] - test[i][d]);
      if ((distance == 0.0f && test[i] != train[n]) || distance != 0.0f && test[i] == train[n])
        fprintf(stderr, "Warning: numerical precision problem. Distance from a point to itself not strictly zero.\n");

      if (points_in_tree && distance == 0.0f) {
        fprintf(stderr, "Forcing distance to inf: %d - %.3f\n", n, distance);
        distance = FLT_MAX;
      }
      nearest[n] = test_kdtree::kd_neighbour(n, distance);
    }

    // Sort the training set vectors by its distance to the test point.
    sort(nearest, nearest + N_train, nearest[0]);

    // Get the K nearest neighbours.
    vector<test_kdtree::kd_neighbour> knn;
    kdtree.knn(test[i], K, knn, epsilon, points_in_tree);

    // Check the k nearest neighbours returned.
    int num_elems = min((int) N_train, (int) K);

    for (int k=0; k<num_elems; ++k) {

      // Check if the points in the tree were correctly ignored if requested.
      if (points_in_tree && test[i] == train[knn[k].index] && nearest[knn[k].index].squared_distance != FLT_MAX) {
        fprintf(stderr, "Nearest neighbour %d failed (in the tree but not ignored): index %d (%.3f), expected index %d (%.3f) in test case %d\n",
          k, nearest[k].index, nearest[k].squared_distance, knn[k].index, knn[k].squared_distance, i);
        ok = false;
      }

      // Check if distances match with expected ones.
      if (fabs(knn[k].squared_distance - nearest[k].squared_distance) >= tolerance) {
        fprintf(stderr, "Nearest neighbour %d failed: index %d (%.3f), expected index %d (%.3f) in test case %d\n",
          k, nearest[k].index, nearest[k].squared_distance, knn[k].index, knn[k].squared_distance, i);
        ok = false;
      }

      // Check if distance calculations are correct.
      float dist1 = sqrt(train[knn[k].index].distance_to(test[i]));
      float dist2 = sqrt(nearest[k].squared_distance);

      if (fabs(dist1 - dist2) >= tolerance) {
        fprintf(stderr, "Nearest neighbour %d failed: returned distance doesn't match (%.6f != %.6f) in test case %d\n",
          k, dist1, dist2, i);
        ok = false;
      }
    }

    // Check vector size.
    if ((int) knn.size() != num_elems) {
      fprintf(stderr, "Wrong nearest neighbour vector size (%d, expected %d) in test case %d\n", (int)knn.size(), num_elems, i);
      ok = false;
    }

    if (!kdtree.verify())
      fprintf(stderr, "Error: invalid kd-tree structure!\n");

    // Calculate a random search range (distance).
    float search_range = (rand() / (float) RAND_MAX + 0.3f) * range;
    float squared_search_range = search_range * search_range;

    // Get all the neighbours in a random range.
    vector<test_kdtree::kd_neighbour> points_in_range;
    kdtree.all_in_range(test[i], search_range, points_in_range, points_in_tree);

    // Check the returned neighbours within the range.
    for (unsigned int k=0; k<points_in_range.size(); ++k) {

      // Check if the points in the tree were correctly ignored if requested.
      if (points_in_tree && test[i] == train[points_in_range[k].index] && nearest[points_in_range[k].index].squared_distance != FLT_MAX) {
          fprintf(stderr, "In-range point : index %d (%.3f) in tree but not ignored in test case %d\n",
          points_in_range[k].index, points_in_range[i].squared_distance, i);
        ok = false;
      }

      // Check point lies within the range.
      if (points_in_range[k].squared_distance > squared_search_range + tolerance) {
        fprintf(stderr, "In-range point failed: index %d (%.3f) greater than requested squared distance %.3f in test case %d\n",
          points_in_range[k].index, points_in_range[i].squared_distance, squared_search_range, i);
        ok = false;
      }

      // Check if returned distances were properly calculated.
      float dist1 = sqrt(train[points_in_range[k].index].distance_to(test[i]));
      float dist2 = sqrt(points_in_range[k].squared_distance);

      if (fabs(dist1 - dist2) >= tolerance) {
        fprintf(stderr, "In-range point failed: returned squared distance doesnt't match (%.6f != %.6f) in test case %d\n",
          dist1, dist2, i);
        ok = false;
      }
    }

    // Count number of points in range.
    unsigned int in_range = 0;
    for (int n=0; n < (int) N_train; ++n) {
      if (nearest[n].squared_distance <= squared_search_range + tolerance) {
        assert(!(points_in_tree && nearest[n].squared_distance == 0));
        ++in_range;
      }
    }

    // Check number of points in range.
    if (in_range != points_in_range.size()) {
      fprintf(stderr, "Wrong number of neighbours within range %.3f (found %d, expected %d) in test case %d\n",
        squared_search_range, static_cast<int>(points_in_range.size()), in_range, i);
      ok = false;
    }
  }

  // TO FIX:.
  // - Case K = training set size.

  // Release random samples and auxiliar data.
  delete []train;
  delete []test;
  delete []nearest;

  // Report results.
  if (ok)
    printf("All tests OK!\n");
  else
    printf("Problems found during testing...\n");

  return 0;
}
