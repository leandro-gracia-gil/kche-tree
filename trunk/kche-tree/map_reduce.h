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
 * \file map_reduce.h
 * \brief Template metaprogramming code for performing bounded and unbounded map-reduce operations with arrays.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_MAPREDUCE_H_
#define _KCHE_TREE_MAPREDUCE_H_

#include "rparam.h"

namespace kche_tree {

// Forward declarations.
template <typename T, unsigned int D, unsigned int i = 0, bool DisableUnroll = (D-i > Settings<T>::max_map_reduce_unroll)> struct MapReduce;
template <typename T, unsigned int D, unsigned int BoundCheckFreq = 1, unsigned int i = 0,
          unsigned int NextCheck = BoundCheckFreq, bool DisableUnroll = (D-i > Settings<T>::max_map_reduce_unroll)> struct BoundedMapReduce;

/**
 * \brief Provide an unrolled generic map-reduce operation over two arrays using template metaprogramming techniques.
 *
 * \tparam T Type of the data in the input arrays.
 * \tparam D Number of dimensions in the input arrays.
 * \tparam i First dimension to operate. Defaults to 0.
 */
template <typename T, unsigned int D, unsigned int i>
struct MapReduce<T, D, i, false> {
  /**
   * \brief Run the mapreduce operation.
   *
   * \param map_reduce Functor used to map and reduce the corresponding elements in the input arrays to a single value.
   * \param a First input array.
   * \param b Second input array.
   * \param accumulator Reference to initial accumulation value and holder of the result when finished.
   */
  template <typename MapReduceFunctor>
  static void run(const MapReduceFunctor &map_reduce, const T *a, const T *b, typename Traits<T>::AccumulatorType &accumulator) {
    MapReduce<T, D, i+1>::run(map_reduce, a, b, map_reduce(accumulator, a, b, i));
  }
};

/// Base case specialization for the map-reduce operation: \a i = \a D. End of arrays case.
template <typename T, unsigned int D>
struct MapReduce<T, D, D, false> {

  /// Nothing needs to be done in this base case.
  template <typename MapReduceFunctor>
  static void run(const MapReduceFunctor &map_reduce, const T *a, const T *b, typename Traits<T>::AccumulatorType &accumulator) {}
};

/// Loop-based version for cases with more dimensions than \link Settings<T>::max_map_reduce_unroll max_map_reduce_unroll\endlink.
template <typename T, unsigned int D, unsigned int i>
struct MapReduce<T, D, i, true> {
  /**
   * \brief Run the map-reduce operation without unrolling loops or using metaprogramming techniques.
   *
   * \param map_reduce Functor used to map and reduce the corresponding elements in the input arrays to a single value.
   * \param a First input array.
   * \param b Second input array.
   * \param accumulator Reference to initial accumulation value and holder of the result when finished.
   */
  template <typename MapReduceFunctor>
  static void run(const MapReduceFunctor &map_reduce, const T *a, const T *b, typename Traits<T>::AccumulatorType &accumulator) {
    for (unsigned int j=i; j<D; ++j)
      map_reduce(accumulator, a, b, j);
  }
};



/**
 * \brief Provide a bounded generic map-reduce operation over two arrays using template metaprogramming techniques.
 * This version of the operation allows to check the boundaries in a specific frequency without the need
 * of runtime conditionals.
 *
 * \tparam T Type of the data in the input arrays.
 * \tparam D Number of dimensions in the input arrays.
 * \tparam BoundCheckFreq Frequency of the boundary checks. Defaults to \c 1, which means to check every 1 dimensions (always). Should never be \c 0.
 * \tparam i First dimension to operate. Defaults to 0.
 * \tparam NextCheck Iterations left until the next boundary check. Defaults to \a BoundCheckFreq.
 */
template <typename T, unsigned int D, unsigned int BoundCheckFreq, unsigned int i, unsigned int NextCheck>
struct BoundedMapReduce<T, D, BoundCheckFreq, i, NextCheck, false> {

  /// Use optimized const reference types.
  typedef typename RParam<T>::Type ConstRef_T;

  /**
   * \brief Run the bounded map-reduce operation.
   *
   * The result may be a partial accumulator if the boundary check passes.
   *
   * \param map_reduce Functor used to map and reduce the corresponding elements in the input arrays to a single value.
   * \param check Functor used for checking the boundaries. Should return \c true when the boundary is crossed. First argument will be the current accumulated value, second will be the \a boundary argument.
   * \param a First input array.
   * \param b Second input array.
   * \param boundary Boundary value used to avoid unnecessary calculations.
   * \param accumulator Reference to initial accumulation value and holder of the result when finished.
   */
  template <typename MapReduceFunctor, typename BoundaryFunctor>
  static void run(const MapReduceFunctor &map_reduce, const BoundaryFunctor& check, const T *a, const T *b, ConstRef_T boundary, typename Traits<T>::AccumulatorType &accumulator) {
    BoundedMapReduce<T, D, BoundCheckFreq, i+1, NextCheck-1>::run(map_reduce, check, a, b, boundary, map_reduce(accumulator, a, b, i));
  }
};

/// Base case specialization for the bounded map-reduce operation: \a NextCheck = \c 0. Perform boundary check.
template <typename T, unsigned int D, unsigned int BoundCheckFreq, unsigned int i>
struct BoundedMapReduce<T, D, BoundCheckFreq, i, 0, false> {

  /// Use optimized const reference types.
  typedef typename RParam<T>::Type ConstRef_T;

  /// Perform the boundary checking with the current value of the accumulator.
  template <typename MapReduceFunctor, typename BoundaryFunctor>
  static void run(const MapReduceFunctor &map_reduce, const BoundaryFunctor &check, const T *a, const T *b, ConstRef_T boundary, typename Traits<T>::AccumulatorType &accumulator) {
    if (check(accumulator, boundary))
      return;
    BoundedMapReduce<T, D, BoundCheckFreq, i, BoundCheckFreq>::run(map_reduce, check, a, b, boundary, accumulator);
  }
};

/// Base case specialization for the bounded map-reduce operation: \a i = \a D. End of arrays.
template <typename T, unsigned int D, unsigned int BoundCheckFreq, unsigned int NextCheck>
struct BoundedMapReduce<T, D, BoundCheckFreq, D, NextCheck, false> {

  /// Use optimized const reference types.
  typedef typename RParam<T>::Type ConstRef_T;

  /// Nothing needs to be done in this base case.
  template <typename MapReduceFunctor, typename BoundaryFunctor>
  static void run(const MapReduceFunctor &map_reduce, const BoundaryFunctor& check, const T *a, const T *b, ConstRef_T boundary, typename Traits<T>::AccumulatorType &accumulator) {}
};

/// Base case specialization for the bounded map-reduce operation: \a i = \a D && NextCheck = \c 0. End of arrays, solves ambiguity.
template <typename T, unsigned int D, unsigned int BoundCheckFreq>
struct BoundedMapReduce<T, D, BoundCheckFreq, D, 0, false> {

  /// Use optimized const reference types.
  typedef typename RParam<T>::Type ConstRef_T;

  /// Nothing needs to be done in this base case.
  template <typename MapReduceFunctor, typename BoundaryFunctor>
  static void run(const MapReduceFunctor &map_reduce, const BoundaryFunctor &check, const T *a, const T *b, ConstRef_T boundary, typename Traits<T>::AccumulatorType &accumulator) {}
};

/// Base case specialization for the bounded map-reduce operation: \a BoundCheckFreq = \c 0. Raise an assertion to avoid an infinite loop.
template <typename T, unsigned int D, unsigned int i, unsigned int nextCheck>
struct BoundedMapReduce<T, D, 0, i, nextCheck, false> {

  /// Use optimized const reference types.
  typedef typename RParam<T>::Type ConstRef_T;

  /// This case should never be reached and contains a false assertion.
  template <typename MapReduceFunctor, typename BoundaryFunctor>
  static void run(const MapReduceFunctor &map_reduce, const BoundaryFunctor &check, const T *a, const T *b, ConstRef_T boundary, typename Traits<T>::AccumulatorType &accumulator) {
    // This case should never be reached. BoundCheckFreq == 0 would lead to an infinite loop.
    assert(0);
  }
};

/// Loop-based version for cases with more dimensions than \link Settings<T>::max_map_reduce_unroll max_map_reduce_unroll\endlink.
template <typename T, unsigned int D, unsigned int BoundCheckFreq, unsigned int i, unsigned int NextCheck>
struct BoundedMapReduce<T, D, BoundCheckFreq, i, NextCheck, true> {

  /// Use optimized const reference types.
  typedef typename RParam<T>::Type ConstRef_T;

  /**
   * \brief Run the bounded map-reduce operation without unrolling loops or using metaprogramming techniques.
   *
   * \param map_reduce Functor used to map and reduce the corresponding elements in the input arrays to a single value.
   * \param check Functor used for checking the boundaries. Should return \c true when the boundary is crossed. First argument will be the current accumulated value, second will be the \a boundary argument.
   * \param a First input array.
   * \param b Second input array.
   * \param boundary Boundary value used to avoid unnecessary calculations.
   * \param accumulator Reference to initial accumulation value and holder of the result when finished.
   */
  template <typename MapReduceFunctor, typename BoundaryFunctor>
  static void run(const MapReduceFunctor &map_reduce, const BoundaryFunctor& check, const T *a, const T *b, ConstRef_T boundary, typename Traits<T>::AccumulatorType &accumulator) {
    unsigned int count = NextCheck;
    for (unsigned int j=i; j<D; ++j) {
      map_reduce(accumulator, a, b, j);
      if (!--count) {
        if (check(accumulator, boundary))
          return;
        count = BoundCheckFreq;
      }
    }
  }
};

} // namespace kche_tree

#endif
