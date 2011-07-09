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
 * \file speed_knn.cpp
 * \brief Speed benchmark tool for the K-Nearest Neighbour search.
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
#include "kche-tree/kche-tree.h"
#endif

// Argument parsing results from gengetopt.
#include "speed_knn_args.h"
static struct speed_knn_args cmdline_args;

// Auxiliary functions for the tools.
#include "tool_utils.h"

// Bring things to the global namespace.
using namespace kche_tree;
using namespace std;

/// Number of dimensions to use in this test.
const unsigned int D = 24;

/// Alias for the specific KDTree and data set types being used.
typedef KDTree<float, D> KDTreeTest;
typedef DataSet<float, D> DataSetTest;
static EuclideanMetric<float, D> metric;

/**
 * Parse command line commands into \a args_info structure (uses gengetopt).
 *
 * \param argc Number of params in command line.
 * \param argv Params of command line.
 * \param args_info Structure to be filled with parsed input params.
*/
void parse_cmdline(int argc, char *argv[], speed_knn_args *args_info) {

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

  // Build the kd-tree.
  clock_t t1_build = clock();
  KDTreeTest kdtree;
  kdtree.build(train_set, cmdline_args.bucket_size_arg);
  clock_t t2_build = clock();

  // Process each test case.
  clock_t t1_test = clock();
  for (unsigned int i=0; i < test_set.size(); ++i) {

    // Get the K nearest neighbours.
    vector<KDTreeTest::NeighbourType> knn;
    if (cmdline_args.use_k_heap_flag)
      kdtree.knn<KHeap>(test_set[i], cmdline_args.knn_arg, knn, metric, cmdline_args.epsilon_arg, cmdline_args.ignore_existing_flag);
    else
      kdtree.knn<KVector>(test_set[i], cmdline_args.knn_arg, knn, metric, cmdline_args.epsilon_arg, cmdline_args.ignore_existing_flag);
  }
  clock_t t2_test = clock();

  // Calculate times.
  double time_build = (t2_build - t1_build) / (double) CLOCKS_PER_SEC;
  double time_test  = (t2_test  - t1_test) / (double) CLOCKS_PER_SEC;

  // Report results.
  printf("Kd-tree testing with D = %d, %d neighbours, epsilon %.2f, training set size %d, test set size %d\n",
      D, cmdline_args.knn_arg, cmdline_args.epsilon_arg, train_set.size(), test_set.size());
  printf("Build time: %.3f sec (%.2f%%)\n", time_build, 100.0 * time_build / (time_build + time_test));
  printf("Test  time: %.3f sec (%.2f%%) -- average per test: %.4f sec\n", time_test , 100.0 * time_test  / (time_build + time_test),
      time_test / (double) test_set.size());

  printf("Total time: %.3f sec\n", time_build + time_test);

  return 0;
}
