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
 * \file verify.cpp
 * \brief Result verification tool for the kche-tree library.
 * \author Leandro Graciá Gil
 */

// Includes from C Standard Library and C++ STL.
#include <cfloat>
#include <cstdlib>
#include <cmath>
#include <ctime>

// Enable kd-tree structure debugging.
#define DEBUG_KDTREE

// Include the generic kche-tree templates or the SSE-enabled specialization for floats and 24 dimensions.
#ifdef SSE
#include "kche-tree_sse_24d.h"
#else
#include "kche-tree/kche-tree.h"
#endif

// Argument parsing results from gengetopt.
#include "verify_args.h"
static struct verify_args cmdline_args;

// Auxiliary functions for the tools.
#include "tool_utils.h"

// Bring things to the global namespace.
using namespace kche_tree;
using namespace std;

/// Number of dimensions used in this test.
const unsigned int D = 24;

/// Alias for the specific KDTree and data set types being used.
typedef KDTree<float, D> KDTreeTest;
typedef DataSet<float, D> DataSetTest;

/**
 * Parse command line commands into \a args_info structure (uses gengetopt).
 *
 * \param argc Number of params in command line.
 * \param argv Params of command line.
 * \param args_info Structure to be filled with parsed input params.
*/
void parse_cmdline(int argc, char *argv[], verify_args *args_info) {

  if (argc == 1) {
    cmdline_parser_print_help();
    exit(0);
  }

  if (cmdline_parser(argc, argv, args_info) != 0) {
    fprintf(stderr, "Run %s --help or -h to see the list of available options.\n", argv[0]);
    exit(1);
  }

  if (args_info->help_given) {
    cmdline_parser_print_help();
    exit(0);
  }

  if (args_info->version_given) {
    cmdline_parser_print_version();
    exit(0);
  }
}

/**
 * Release the strings allocated during the command line parsing.
 */
void release_cmdline() {
  cmdline_parser_free(&cmdline_args);
}

/**
 * Application entry point.
 */
int main(int argc, char *argv[]) {

  // Parse the command line.
  parse_cmdline(argc, argv, &cmdline_args);
  atexit(release_cmdline);

  // Validate the arguments.
  if (cmdline_args.knn_arg < 0) {
    fprintf(stderr, "Invalid knn value.\n");
    return 1;
  }

  if (cmdline_args.all_in_range_arg < 0) {
    fprintf(stderr, "Invalid all-in-range value.\n");
    return 1;
  }

  if (cmdline_args.train_random_given && cmdline_args.train_random_arg <= 0) {
    fprintf(stderr, "Invalid random train set size.\n");
    return 1;
  }

  if (cmdline_args.test_random_given && cmdline_args.test_random_arg <= 0) {
    fprintf(stderr, "Invalid random test set size.\n");
    return 1;
  }

  if (cmdline_args.bucket_size_arg <= 0) {
    fprintf(stderr, "Invalid bucket size.\n");
    return 1;
  }

  if (cmdline_args.tolerance_arg < 0.0f) {
    fprintf(stderr, "Invalid tolerance value.\n");
    return 1;
  }

  if (cmdline_args.test_from_train_arg < 0.0f || cmdline_args.test_from_train_arg > 100.0f) {
    fprintf(stderr, "Invalid test-from-train value. Should be between 0 and 100%%.\n");
    return 1;
  }

  if (cmdline_args.epsilon_arg < 0.0f) {
    fprintf(stderr, "Invalid test-from-train value. Should be between 0 and 100%%.\n");
    return 1;
  }

  // Initialize the random seed.
  if (cmdline_args.random_seed_given)
    srand(cmdline_args.random_seed_arg);
  else
    srand(time(NULL));

  // Get the train set data using the given command line options.
  DataSetTest train_set;
  if (!get_train_set_data(cmdline_args, train_set))
    return 1;

  // Get the test set data using the given command line options.
  DataSetTest test_set;
  if (!get_test_set_data(cmdline_args, train_set, test_set))
    return 1;

  // Define the global result flag.
  bool ok = true;

  // Build the kd-tree.
  KDTreeTest kdtree;
  kdtree.build(train_set, cmdline_args.bucket_size_arg);

  // Verify the properties of the tree.
  if (!kdtree.verify()) {
    fprintf(stderr, "Error building the tree: the result does not satisfy the properties of a kd-tree.\n");
    return 1;
  }

  // Test the kd-tree I/O.
  if (cmdline_args.kdtree_io_flag) {

    // Try to save the kd-tree to a file.
    ofstream out_file("kd-tree.test", ios::binary | ios::out);
    out_file << kdtree;
    out_file.close();

    // Read the tree back.
    ifstream in_file("kd-tree.test", ios::binary | ios::in);
    in_file >> kdtree;
    in_file.close();

    // Verify the properties of the kd-tree just read.
    if (!kdtree.verify()) {
      fprintf(stderr, "Kd-tree I/O failure: the tree read from file does not satisfy the properties of a kd-tree.\n");
      ok = false;
    }
  }

  // Test the subscript operator.
  if (cmdline_args.subscript_flag) {
    for (unsigned int i=0; i<train_set.size(); ++i) {
      if (train_set[i] != kdtree[i]) {
        fprintf(stderr, "Non-matching subscript operator value for index %d (error in 1st dimension: %.6f).\n", i, fabs(train_set[i][0] - kdtree[i][0]));
        ok = false;
      }
    }
  }

  // Allocate memory for the exhaustive calculation of nearest neighbours.
  KDTreeTest::Neighbour *nearest = new KDTreeTest::Neighbour[train_set.size()];
  assert(nearest);

  // Process each test case.
  for (unsigned int i=0; i<test_set.size(); ++i) {

    // Create a vector of point-distance tuples to current test point.
    if (cmdline_args.knn_arg > 0 || cmdline_args.all_in_range_arg > 0.0f) {
      for (unsigned int n=0; n<train_set.size(); ++n) {
        float distance = 0.0f;
        for (unsigned int d=0; d<D; ++d)
          distance += (train_set[n][d] - test_set[i][d]) * (train_set[n][d] - test_set[i][d]);

        // Check numerical consistency of the distance.
        if ((distance == 0.0f && test_set[i] != train_set[n]) || (distance != 0.0f && test_set[i] == train_set[n]))
          fprintf(stderr, "Warning: possible numerical precision problem. Distance from a point to itself not strictly zero.\n");

        // Exclude the point from the nearest neighbours as knn will do.
        if (cmdline_args.ignore_existing_flag && distance == 0.0f)
          distance = FLT_MAX;

        nearest[n] = KDTreeTest::Neighbour(n, distance);
      }
    }

    // Sort the training set vectors by its distance to the test point.
    sort(nearest, nearest + train_set.size(), nearest[0]);

    // Test the knn functionality.
    if (cmdline_args.knn_arg > 0) {

      // Get the K nearest neighbours.
      unsigned int K = cmdline_args.knn_arg;
      vector<KDTreeTest::Neighbour> knn;
      kdtree.knn(test_set[i], K, knn, cmdline_args.epsilon_arg, cmdline_args.ignore_existing_flag);

      // Check the k nearest neighbours returned.
      unsigned int num_elems = min(static_cast<unsigned int>(knn.size()), K);
      for (unsigned int k=0; k<num_elems; ++k) {

        // Check if the points in the tree were correctly ignored if requested.
        if (cmdline_args.ignore_existing_flag && test_set[i] == train_set[knn[k].index] && nearest[knn[k].index].squared_distance != FLT_MAX) {
          fprintf(stderr, "Nearest neighbour %d failed (in the tree but not ignored): index %d (%.3f), expected index %d (%.3f) in test case %d\n",
            k, nearest[k].index, nearest[k].squared_distance, knn[k].index, knn[k].squared_distance, i);
          ok = false;
        }

        // Check if distances match with expected ones.
        if (fabs(knn[k].squared_distance - nearest[k].squared_distance) >= cmdline_args.tolerance_arg) {
          fprintf(stderr, "Nearest neighbour %d failed: index %d (%.3f), expected index %d (%.3f) in test case %d\n",
            k, nearest[k].index, nearest[k].squared_distance, knn[k].index, knn[k].squared_distance, i);
          ok = false;
        }

        // Check if distance calculations are correct within a tolerance value.
        // Since the knn method uses incremental calculations and floating point arithmetic is not really commutative,
        // the values are expected to be slightly different, possibly more with higher dimension values.
        float dist1 = sqrt(train_set[knn[k].index].distance_to(test_set[i]));
        float dist2 = sqrt(nearest[k].squared_distance);

        if (fabs(dist1 - dist2) >= cmdline_args.tolerance_arg) {
          fprintf(stderr, "Nearest neighbour %d failed: returned distance doesn't match (%.6f != %.6f) in test case %d\n",
            k, dist1, dist2, i);
          ok = false;
        }
      }

      // Check vector size.
      if (knn.size() != num_elems) {
        fprintf(stderr, "Wrong nearest neighbour vector size (%zu, expected %d) in test case %d\n", knn.size(), num_elems, i);
        ok = false;
      }
    }

    // Test the all-in-range functionality.
    if (cmdline_args.all_in_range_arg > 0.0f) {

      // Calculate the squared range search distance.
      float search_range = cmdline_args.all_in_range_arg;
      float squared_search_range = search_range * search_range;

      // Get all the neighbours in a random range.
      vector<KDTreeTest::Neighbour> points_in_range;
      kdtree.all_in_range(test_set[i], search_range, points_in_range, cmdline_args.ignore_existing_flag);

      // Check the returned neighbours within the range.
      for (unsigned int k=0; k<points_in_range.size(); ++k) {

        // Check if the points in the tree were correctly ignored if requested.
        if (cmdline_args.ignore_existing_flag && test_set[i] == train_set[points_in_range[k].index] &&
            nearest[points_in_range[k].index].squared_distance != FLT_MAX) {
          fprintf(stderr, "In-range point : index %d (%.3f) in tree but not ignored in test case %d\n",
            points_in_range[k].index, points_in_range[i].squared_distance, i);
          ok = false;
        }

        // Check point lies within the range.
        if (points_in_range[k].squared_distance > squared_search_range + cmdline_args.tolerance_arg) {
          fprintf(stderr, "In-range point failed: index %d (%.3f) greater than requested squared distance %.3f in test case %d\n",
            points_in_range[k].index, points_in_range[i].squared_distance, squared_search_range, i);
          ok = false;
        }

        // Check if returned distances were properly calculated.
        float dist1 = sqrt(train_set[points_in_range[k].index].distance_to(test_set[i]));
        float dist2 = sqrt(points_in_range[k].squared_distance);

        if (fabs(dist1 - dist2) >= cmdline_args.tolerance_arg) {
          fprintf(stderr, "In-range point failed: returned squared distance doesnt't match (%.6f != %.6f) in test case %d\n", dist1, dist2, i);
          ok = false;
        }
      }

      // Count number of points in range.
      unsigned int in_range = 0;
      for (unsigned int n=0; n<train_set.size(); ++n) {
        if (nearest[n].squared_distance <= squared_search_range + cmdline_args.tolerance_arg) {
          assert(!(cmdline_args.ignore_existing_flag && nearest[n].squared_distance == 0));
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
  }

  // Release auxiliar neighbour data.
  delete []nearest;

  // Report results.
  if (ok)
    cout << "All tests OK!" << endl;
  else
    cout << "Problems found during testing..." << endl;

  return 0;
}
