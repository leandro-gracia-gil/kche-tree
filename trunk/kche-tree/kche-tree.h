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
 * - Automatic SSE code generation and unrolling optimized for the requested number of dimensions.
 * - Internal data permutation to increase cache hits.
 * - Contiguous bucket data to reduce the leaf node size.
 * - Preorder memory allocation during the build to increase cache hits when traversing.
 * - Exploration/intersection recursive scheme to reduce the number of calculations performed.
 * - Use of specific k-neighbours optimized containers: k-vectors and k-heaps.
 * - Distance calculations with upper bounds allowing early returns.
 * - Binary file format and stream operators provide to easily save and load the kd-trees and data sets.
 *
 * The current version is not still thread-safe. This is expected to be solved in future releases
 * along with OpenMP optimizations.
 *
 * Additionally, the following tools and examples are provided:
 * - <b>Verifation tools</b>: set of tools to verify the correction of the results provided by Kche-tree compared with a raw exhaustive search.
 *   Automatically generated for the metric, dimensions and type specified in tools/Makefile.tools.
 * - <b>Benchmark tool</b>: set of tools to measure the time spent in building the kd-tree and finding the K nearest neighbours.
 *   Automatically generated for the metric, dimensions and type specified in tools/Makefile.tools.
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
 * \section settings Settings
 * Kche-tree allows some extra settings to be set by defining macros before including the main header.
 * Some of these options include:
 * - \c KCHE_TREE_ENABLE_SSE: enables using SSE instructions to perform the distance calculations.
 *   Will only have effect if SSE is supported and enabled by the compiler (remember to compile using any required flags!).
 *   Defaults to \c false.
 * - \c KCHE_TREE_MAX_UNROLL: the maximum number of dimensions for which loop unrolling is allowed.
 *   Lower this value to disable loop unrolling or to avoid compiler errors due to the maximum template recursion limit.
 *   Defaults to 1024.
 * - \c KCHE_TREE_VERIFY_KDTREE_AFTER_DESERIALIZING: if enabled, the structural properties of the kd-tree are verified after loading it from a file.
 *   This can be disabled for performance reasons if set to \c false. Defaults to \c true.
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

// Macros used to recognize SSE compiler settings.
#if defined(__SSE__) || defined(__SSE2__) || (defined(_M_IX86_FP) && (_M_IX86_FP >= 1))
#define KCHE_TREE_SSE_SUPPORTED true
#if defined(__SSE2__) || (defined(_M_IX86_FP) && (_M_IX86_FP >= 2))
#define KCHE_TREE_SSE2_SUPPORTED true
#endif
#else
#define KCHE_TREE_SSE_SUPPORTED false
#define KCHE_TREE_SSE2_SUPPORTED false
#endif

// Default values for the settings.
#if !defined(KCHE_TREE_MAX_UNROLL)
#define KCHE_TREE_MAX_UNROLL 1024
#endif

#if !defined(KCHE_TREE_VERIFY_KDTREE_AFTER_DESERIALIZING)
#define KCHE_TREE_VERIFY_KDTREE_AFTER_DESERIALIZING true
#endif

#if !defined(KCHE_TREE_ENABLE_SSE)
#define KCHE_TREE_ENABLE_SSE false
#endif

// Disable the SSE enable macro if not supported
#if (KCHE_TREE_ENABLE_SSE) && !(KCHE_TREE_SSE_SUPPORTED)
#undef KCHE_TREE_ENABLE_SSE
#define KCHE_TREE_ENABLE_SSE false
#endif

// Debug assertion macros.
#if defined(KCHE_TREE_DEBUG)
#define KCHE_TREE_DCHECK(cond) assert(cond)
#else
#define KCHE_TREE_DCHECK(cond)
#endif

// Code not reached macros.
#define KCHE_TREE_NOT_REACHED() assert(false)

// Forward-declare data sets and vectors.
template <typename T, const unsigned int D> class DataSet;
template <typename T, const unsigned int D> class Vector;

/**
 * \brief Compile-time settings for the Kche-tree library.
 *
 * Defines various options to enhance the library functionality
 * at compile time. Specialize the template to set new values for them.
 */
struct Settings {
  /// Maximum number of dimensions to unroll when using map-reduce operations. If exceeded a loop will be used instead.
  /// \warning Increasing this value may cause compile errors due to the compiler template recursion limit.
  static const unsigned int max_map_reduce_unroll = KCHE_TREE_MAX_UNROLL;

  /// Check if the kd-tree structure should be verified when deserializing. Will not compile the verification code if disabled.
  static const bool verify_kdtree_after_deserializing = KCHE_TREE_VERIFY_KDTREE_AFTER_DESERIALIZING;

  /// Enable SSE optimizations. Any specific required flags are assumed to be passed to the compiler.
  /// \warning This only works with some metrics, types (including accumulator types) and the number of dimensions should be a multiple of 4.
  static const bool enable_sse = KCHE_TREE_ENABLE_SSE;
};

/**
 * \brief Compile-time type settings for the Kche-tree library.
 *
 * Defines the main types vector and data set types to be used by the different
 * classes of the library. Specialize the template to redefine.
 *
 * \tparam Type of the elements used by the library to which the options apply.
 * \tparam D Number of dimensions used by the library to which the options apply.
 */
template <typename T, const unsigned int D>
struct TypeSettings {
  /// Type of the data set to use.
  typedef DataSet<T, D> DataSetType;

  /// Type of the vectors to be used.
  typedef Vector<T, D> VectorType;
};

} // namespace kche_tree

// Include the kd-tree template.
#include "kd-tree.h"

#endif
