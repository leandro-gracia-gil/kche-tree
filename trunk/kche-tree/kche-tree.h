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
 * \mainpage Kche-tree Template Documentation
 *
 * \section introduction Introduction
 * Kche-tree is a template for generic cache-aware and non-mutable kd-trees.
 * Its main purpose is to provide an easy to use but powerful implementation
 * of the typical kd-tree structure functionality with very low latencies.
 *
 * It provides the following basic operations:
 * - \link kche_tree::KDTree::build Build\endlink: create a kd-tree from a training set of
 *   feature vectors. Median splitting is used to keep the tree balanced. Cost: O(n log² n).
 * - \link kche_tree::KDTree::knn K nearest neighbours\endlink: retrieve the K nearest
 *   neighbours of a given feature vector. Estimated average cost: O(log K log n).
 * - \link kche_tree::KDTree::all_in_range All neighbours within a range\endlink: retrieve all the
 *   neighbours inside a maximum distance radius from a given feature vector.
 *   Estimated average cost: O(log m log n) with \e m the number of neighbours in the range.
 *
 * The template has been designed to minimize the number of cache misses combined
 * with many algorithmic techniques and ideas.\n
 * Here are some of its features:
 * - Incremental calculation of the hyperrectangle intersections.
 * - Can define the metrics to use when exploring the tree: Euclidean, Mahalanobis, Chebyshev, etc.
 * - Internal data permutation to increase cache hits.
 * - Contiguous bucket data to reduce the leaf node size.
 * - Preorder memory allocation during the build to increase cache hits when traversing.
 * - Exploration/intersection recursive scheme to reduce the number of calculations performed.
 * - Use of specific k-neighbours optimized containers: k-vectors and k-heaps.
 * - Distance calculations with upper bounds allowing early returns.
 * - Designed to easily enable SSE optimizations by template specialization. Example and benchmarks included.
 * - Binary file format and stream operators provide to easily save and load the kd-trees and data sets.
 *
 * The current version is not still thread-safe. This is expected to be solved in future releases
 * along with OpenMP optimizations.
 *
 * Additionally, the following tools and examples are provided:
 * - <b>Verify tool</b>: verifies the correction of the results provided by Kche-tree compared with a raw exhaustive search.
 *   Can be found in the file tools/verify.cpp. Comes with an extra SSE-optimized version.
 * - <b>Benchmark tool</b>: measures the time spent in building the kd-tree and finding the K nearest neighbours.
 *   Can be found in file speed_kdtree.cpp. Like the testing tool it also comes comes with an extra SSE-optimized version.
 * - <b>knn simple example</b>: shows a quite simple use of the template. Can be found in the file examples/knn_simple.cpp.
 * - <b>custom types example</b>: shows how to use a custom type with Kche-tree. Can be found in the file examples/custom_type.cpp.
 *
 *
 * \section usage Usage
 * Kche-trees are quite simple to use. Just include the \c kche-tree.h file, use its
 * namespace like you would normally do with STL and proceed like in the usage example \c knn_simple.cpp.
 * Since it's a template no additional files required to be compiled or linked. However it requires
 * the \c kche-tree folder to be present in an include path. This can also be achieved by running \c make \c install,
 * will copy the template files to your /usr/include folder by default. To undo this run \c make \c uninstall.
 *
 * In case of using the floating point/24 dimension SSE specialization,
 * the file tools/kd-tree_sse_24d.h and the compilation flag \c -msse (in GCC) will be also required.
 *
 * \section CPP0x About C++0x
 * Kche-trees use by default C++0x features available in the most modern compilers to enhance its use and operations.
 * If C++0x is not supported by your compiler it can be disabled by simply defining the macro \c KCHE_TREE_DISABLE_CPP0X before including the library.
 * Additionally it can be disabled when building the examples and tools by using \c make \c disable=c++0x.
 *
 * However, be aware that disabling C++0x has some consequences:
 * - When calling the \link kche_tree::KDTree::knn knn\endlink method it will be required to explicitly provide the K-Neighbour container template type, where in the C++0x version it automatically defaults to \link kche_tree::KVector KVector\endlink. See the \c knn_simple example for details.
 * - Any static assert that the library may make will leave no explanation message, just some kind of compiler error mentioning COMPILE_ASSERT_FAILURE.
 * - Some type traits might not be available, leading to possible misses of automatic optimization chances.
 *
 * Expect the previous list to grow and the possible future addition of C++0x-only features. It is also possible that the support for non-C++0x code can be completely removed in future releases.
*/

/**
 * \file kche-tree.h
 * \brief Main kche-tree library header.
 * \author Leandro Graciá Gil
*/

#ifndef _KCHE_TREE_KCHE_TREE_H_
#define _KCHE_TREE_KCHE_TREE_H_

/// Namespace of the Kche-tree template library.
namespace kche_tree {

// Forward-declare data sets and vectors.
template <typename T, const unsigned int D> class DataSet;
template <typename T, const unsigned int D> class Vector;

/**
 * \brief Compile-time settings for the Kche-tree library.
 *
 * Defines the main types to be used by the different classes of the library
 * and enables some options. Override or specialize to change them.
 *
 * \tparam Type of the elements in the provided data.
 * \tparam D Number of dimensions of the provided data.
 */
template <typename T, const unsigned int D>
struct Settings {
  /// Type of the data set to use.
  typedef DataSet<T, D> DataSetType;

  /// Type of the vectors to be used.
  typedef Vector<T, D> VectorType;

  /// Check if the kd-tree structure should be verified when deserializing. Will not compile the verification code if disabled.
  static const bool verify_kdtree_after_deserializing = true;
};

} // namespace kche_tree

// Required library includes.
#include "kd-tree.h"

#endif