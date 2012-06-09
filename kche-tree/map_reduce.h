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

// Include STL binary functions and comparison functors.
#include <functional>

// Include metaprogramming utilities.
#include "utils.h"

// Include map-reduce functor concepts.
#include "map_reduce_functor.h"


namespace kche_tree {

// Forward declarations.
template <typename T, unsigned int D, unsigned int Index = 0, unsigned int D_max = D, unsigned int BlockSize = 1, bool DisableUnroll = (D_max - Index > Settings::max_map_reduce_unroll)>
struct MapReduce;

template <unsigned int BoundCheckFreq, typename T, unsigned int D = 1, unsigned int Index = 0, unsigned int D_max = D, unsigned int BlockSize = 1,
          unsigned int NextCheck = BoundCheckFreq, bool DisableUnroll = (D_max - Index > Settings::max_map_reduce_unroll)>
struct BoundedMapReduce;

/**
 * \brief Provide an unrolled generic map-reduce operation over two arrays using template metaprogramming techniques.
 *
 * Performs a map and dimensionality reduction operation for a set of input arrays.
 *
 * \tparam T Type of the data in the input arrays.
 * \tparam D Number of dimensions in the input arrays.
 * \tparam Index First dimension to operate. Defaults to 0.
 * \tparam D_max Last dimension (non-inclusive) to operate. Defaults to \a D.
 * \tparam BlockSize Number of consecutive dimensions to process in each call to the functor. Defaults to \c 1.
 */
template <typename T, unsigned int D, unsigned int Index, unsigned int D_max, unsigned int BlockSize>
struct MapReduce<T, D, Index, D_max, BlockSize, false> {
  /**
   * \brief Run the mapreduce operation.
   *
   * \param map_reduce Functor used to map and reduce the corresponding elements in the input arrays to a single value.
   * \param accumulator Reference to initial accumulation value and holder of the result when finished.
   * \param a First input array.
   * \param b Second input array. Optional.
   * \param extra Extra arguments provided to the map-reduce function. Optional.
   */
  template <typename MapReduceFunctor, typename Accumulator>
  static inline void run(const MapReduceFunctor &map_reduce, Accumulator &accumulator, const T *a, const T *b = NULL, const void *extra = NULL) {
    KCHE_TREE_CHECK_CONCEPT(MapReduceFunctor, MapReduceFunctorConcept<T>);
    static const unsigned int StepSize = Min<BlockSize, D_max - Index>::value;
    static const unsigned int NextStep = Min<StepSize, D_max - Index - StepSize>::value;
    MapReduce<T, D, Index + StepSize, D_max, NextStep>::run(map_reduce, map_reduce.template operator () <Index, StepSize, D>(accumulator, a, b, extra), a, b, extra);
  }
};

/// Base case specialization for the map-reduce operation: \a Index = \a D_max. End of arrays case.
template <typename T, unsigned int D, unsigned int D_max, unsigned int BlockSize>
struct MapReduce<T, D, D_max, D_max, BlockSize, false> {

  /// Nothing needs to be done in this base case.
  template <typename MapReduceFunctor, typename Accumulator>
  static inline void run(const MapReduceFunctor &map_reduce, Accumulator &accumulator, const T *a, const T *b = NULL, const void *extra = NULL) {
    KCHE_TREE_CHECK_CONCEPT(MapReduceFunctor, MapReduceFunctorConcept<T>);
  }
};

/**
 * \brief Run the map-reduce operation without unrolling loops or using metaprogramming techniques.
 *
 * \param map_reduce Functor used to map and reduce the corresponding elements in the input arrays to a single value.
 * \param accumulator Reference to initial accumulation value and holder of the result when finished.
 * \param a First input array.
 * \param b Second input array. Optional, defaults to \c NULL.
 * \param extra Extra arguments provided to the map-reduce functor. Optional, defaults to \c NULL.
 * \param i Index of the first dimension to operate. Optional, defaults to \c 0.
 * \param i_max Index of the last (non-inclusive) dimension to operate. Optional, defaults to \a D.
 * \param block_size Number of consecutive dimensions to process in each call to \a map_reduce. Optional, defaults to \c 1.
 */
template <typename T, unsigned int D, typename MapReduceFunctor, typename Accumulator>
inline void non_unrolled_map_reduce(const MapReduceFunctor &map_reduce, Accumulator &accumulator, const T *a, const T *b = NULL, const void *extra = NULL, unsigned int i = 0, unsigned int i_max = D, unsigned int block_size = 1) {
  KCHE_TREE_CHECK_CONCEPT(MapReduceFunctor, MapReduceFunctorConcept<T>);
  while (i < i_max) {
    unsigned int step_size = std::min(block_size, i_max - i);
    map_reduce.template operator () <D>(i, step_size, accumulator, a, b, extra);
    i += step_size;
  }
}

/// Loop-based version for cases with more dimensions than \link Settings<T>::max_map_reduce_unroll max_map_reduce_unroll\endlink.
template <typename T, unsigned int D, unsigned int Index, unsigned int D_max, unsigned int BlockSize>
struct MapReduce<T, D, Index, D_max, BlockSize, true> {
  /// Run the loop-based version of the map reduce operation.
  template <typename MapReduceFunctor, typename Accumulator>
  static inline void run(const MapReduceFunctor &map_reduce, Accumulator &accumulator, const T *a, const T *b = NULL, const void *extra = NULL) {
    non_unrolled_map_reduce<T, D>(map_reduce, accumulator, a, b, extra, Index, D_max, BlockSize);
  }
};




/**
 * \brief Provide a bounded generic map-reduce operation over two arrays using template metaprogramming techniques.
 * This version of the operation allows to check the boundaries in a specific frequency without the need
 * of runtime conditionals.
 *
 * \tparam BoundCheckFreq Frequency of the boundary checks. Defaults to \c 1, which means to check every 1 dimensions (always). Should never be \c 0.
 * \tparam T Type of the data in the input arrays.
 * \tparam D Number of dimensions in the input arrays.
 * \tparam Index First dimension to operate. Defaults to 0.
 * \tparam D_max Index of the last dimension (non-inclusive) to operate.
 * \tparam BlockSize Number of consecutive dimensions to process in each call to the functor.
 * \tparam NextCheck Iterations left until the next boundary check. Defaults to \a BoundCheckFreq.
 */
template <unsigned int BoundCheckFreq, typename T, unsigned int D, unsigned int Index, unsigned int D_max, unsigned int BlockSize, unsigned int NextCheck>
struct BoundedMapReduce<BoundCheckFreq, T, D, Index, D_max, BlockSize, NextCheck, false> {

  /**
   * \brief Run the bounded map-reduce operation.
   *
   * The result may be a partial accumulator if the boundary check passes.
   *
   * \param map_reduce Functor used to map and reduce the corresponding elements in the input arrays to a single value.
   * \param accumulator Reference to initial accumulation value and holder of the result when finished.
   * \param check Functor used for checking the boundaries. Should return \c true when the boundary is crossed. First argument will be the current accumulated value, second will be the \a boundary argument.
   * \param boundary Boundary value used to avoid unnecessary calculations. Must implement std::binary_function<T, T, bool>.
   * \param a First input array.
   * \param b Second input array.
   * \param extra Extra arguments provided to the map-reduce functor. Optional, defaults to \c NULL.
   */
  template <typename MapReduceFunctor, typename Accumulator, typename BoundaryCheckFunctor, typename Boundary>
  static inline void run(const MapReduceFunctor &map_reduce, Accumulator &accumulator, const BoundaryCheckFunctor& check, const Boundary &boundary, const T *a, const T *b = NULL, const void *extra = NULL) {
    KCHE_TREE_CHECK_CONCEPT(MapReduceFunctor, MapReduceFunctorConcept<T>);
    KCHE_TREE_CHECK_CONCEPT(BoundaryCheckFunctor, BoundaryCheckFunctorConcept<Boundary>);
    static const unsigned int StepSize = Min<BlockSize, D_max - Index>::value;
    static const unsigned int NextStep = Min<StepSize, D_max - Index - StepSize>::value;
    BoundedMapReduce<BoundCheckFreq, T, D, Index + StepSize, D_max, NextStep, NextCheck - 1>::run(map_reduce, map_reduce.template operator () <Index, StepSize, D>(accumulator, a, b, extra), check, boundary, a, b, extra);
  }
};

/// Base case specialization for the bounded map-reduce operation: \a NextCheck = \c 0. Perform boundary check.
template <unsigned int BoundCheckFreq, typename T, unsigned int D, unsigned int Index, unsigned int D_max, unsigned int BlockSize>
struct BoundedMapReduce<BoundCheckFreq, T, D, Index, D_max, BlockSize, 0, false> {

  /// Perform the boundary checking with the current value of the accumulator.
  template <typename MapReduceFunctor, typename Accumulator, typename BoundaryCheckFunctor, typename Boundary>
  static inline void run(const MapReduceFunctor &map_reduce, Accumulator &accumulator, const BoundaryCheckFunctor &check, const Boundary &boundary, const T *a, const T *b = NULL, const void *extra = NULL) {
    KCHE_TREE_CHECK_CONCEPT(MapReduceFunctor, MapReduceFunctorConcept<T>);
    KCHE_TREE_CHECK_CONCEPT(BoundaryCheckFunctor, BoundaryCheckFunctorConcept<Boundary>);
    if (check(accumulator, boundary))
      return;
    BoundedMapReduce<BoundCheckFreq, T, D, Index, D_max, BlockSize, BoundCheckFreq>::run(map_reduce, accumulator, check, boundary, a, b, extra);
  }
};

/// Base case specialization for the bounded map-reduce operation: \a Index = \a D_max. End of arrays.
template <unsigned int BoundCheckFreq, typename T, unsigned int D, unsigned int D_max, unsigned int NextCheck>
struct BoundedMapReduce<BoundCheckFreq, T, D, D_max, D_max, 0, NextCheck, false> {

  /// Nothing needs to be done in this base case.
  template <typename MapReduceFunctor, typename Accumulator, typename BoundaryCheckFunctor, typename Boundary>
  static inline void run(const MapReduceFunctor &map_reduce, Accumulator &accumulator, const BoundaryCheckFunctor& check, const Boundary &boundary, const T *a, const T *b = NULL, const void *extra = NULL) {
    KCHE_TREE_CHECK_CONCEPT(MapReduceFunctor, MapReduceFunctorConcept<T>);
    KCHE_TREE_CHECK_CONCEPT(BoundaryCheckFunctor, BoundaryCheckFunctorConcept<Boundary>);
  }
};

/// Base case specialization for the bounded map-reduce operation: \a Index = \a D_max && NextCheck = \c 0. End of arrays, solves ambiguity.
template <unsigned int BoundCheckFreq, typename T, unsigned int D, unsigned int D_max>
struct BoundedMapReduce<BoundCheckFreq, T, D, D_max, D_max, 0, 0, false> {

  /// Nothing needs to be done in this base case.
  template <typename MapReduceFunctor, typename Accumulator, typename BoundaryCheckFunctor, typename Boundary>
  static inline void run(const MapReduceFunctor &map_reduce, Accumulator &accumulator, const BoundaryCheckFunctor &check, const Boundary &boundary, const T *a, const T *b = NULL, const void *extra = NULL) {
    KCHE_TREE_CHECK_CONCEPT(MapReduceFunctor, MapReduceFunctorConcept<T>);
    KCHE_TREE_CHECK_CONCEPT(BoundaryCheckFunctor, BoundaryCheckFunctorConcept<Boundary>);
  }
};

/// Base case specialization for the bounded map-reduce operation: \a BoundCheckFreq = \c 0. Raise an assertion to avoid an infinite loop.
template <typename T, unsigned int D, unsigned int Index, unsigned int D_max, unsigned int BlockSize, unsigned int NextCheck>
struct BoundedMapReduce<0, T, D, Index, D_max, BlockSize, NextCheck, false> {

  /// This case should never be reached and contains a false assertion.
  template <typename MapReduceFunctor, typename Accumulator, typename BoundaryCheckFunctor, typename Boundary>
  static inline void run(const MapReduceFunctor &map_reduce, Accumulator &accumulator, const BoundaryCheckFunctor &check, const Boundary &boundary, const T *a, const T *b = NULL, const void *extra = NULL) {
    KCHE_TREE_CHECK_CONCEPT(MapReduceFunctor, MapReduceFunctorConcept<T>);
    KCHE_TREE_CHECK_CONCEPT(BoundaryCheckFunctor, BoundaryCheckFunctorConcept<Boundary>);

    // This case should never be reached. BoundCheckFreq == 0 would lead to an infinite loop.
    KCHE_TREE_NOT_REACHED();
  }
};

/**
 * \brief Run the bounded map-reduce operation without unrolling loops or using metaprogramming techniques.
 *
 * \tparam BoundCheckFreq Integer defining the frequency of the boundary checks.
 *
 * \param map_reduce Functor used to map the input and reduce the corresponding elements in the input arrays to a single value.
 * \param accumulator Reference to initial accumulation value and holder of the result when finished.
 * \param check Functor used for checking the boundaries. Should return \c true when the boundary is crossed. First argument will be the current accumulated value, second will be the \a boundary argument.
 * \param boundary Boundary value used to avoid unnecessary calculations.
 * \param a First input array.
 * \param b Second input array. Optional, defaults to \c NULL.
 * \param extra Extra arguments provided to the map-reduce function. Optional, defaults to \c NULL.
 * \param i Index of the first dimension to operate. Optional, defaults to \c 0.
 * \param i_max Index of the last (non-inclusive) dimension to operate. Optional, defaults to \a D.
 * \param block_size Number of consecutive dimensions to process in each call to \a map_reduce. Optional, defaults to \c 1.
 * \param next_check Number of iterations left before the next boundary check.
 */
template <unsigned int BoundCheckFreq, typename T, unsigned int D, typename MapReduceFunctor, typename Accumulator, typename BoundaryCheckFunctor, typename Boundary>
inline void non_unrolled_bounded_map_reduce(const MapReduceFunctor &map_reduce, Accumulator &accumulator, const BoundaryCheckFunctor &check, const Boundary &boundary, const T *a, const T *b = NULL, const void *extra = NULL, unsigned int i = 0, unsigned int i_max = D, unsigned int block_size = 1, unsigned int next_check = BoundCheckFreq) {
  KCHE_TREE_CHECK_CONCEPT(MapReduceFunctor, MapReduceFunctorConcept<T>);
  KCHE_TREE_CHECK_CONCEPT(BoundaryCheckFunctor, BoundaryCheckFunctorConcept<Boundary>);

  unsigned int count = next_check;
  while (i < i_max) {
    unsigned int step_size = std::min(block_size, i_max - i);
    map_reduce.template operator () <D>(i, step_size, accumulator, a, b, extra);
    if (!--count) {
      if (check(accumulator, boundary))
        return;
      count = BoundCheckFreq;
    }
    i += step_size;
  }
}

/// Loop-based version for cases with more dimensions than \link Settings<T>::max_map_reduce_unroll max_map_reduce_unroll\endlink.
template <unsigned int BoundCheckFreq, typename T, unsigned int D, unsigned int Index, unsigned int D_max, unsigned int BlockSize, unsigned int NextCheck>
struct BoundedMapReduce<BoundCheckFreq, T, D, Index, D_max, BlockSize, NextCheck, true> {

  template <typename MapReduceFunctor, typename Accumulator, typename BoundaryCheckFunctor, typename Boundary>
  static inline void run(const MapReduceFunctor &map_reduce, Accumulator &accumulator, const BoundaryCheckFunctor &check, const Boundary &boundary, const T *a, const T *b = NULL, const void *extra = NULL) {
    non_unrolled_bounded_map_reduce<BoundCheckFreq, T, D>(map_reduce, accumulator, check, boundary, a, b, extra, Index, D_max, BlockSize, NextCheck);
  }
};

} // namespace kche_tree

#endif
