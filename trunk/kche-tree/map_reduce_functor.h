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
 * \file map_reduce_functor.h
 * \brief Concept definitions for map-reduce functors.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_MAPREDUCE_FUNCTOR_H_
#define _KCHE_TREE_MAPREDUCE_FUNCTOR_H_

// Include STL binary functions and comparison functors.
#include <functional>

// Include metaprogramming utilities.
#include "utils.h"

namespace kche_tree {

/**
 * \brief Base concept to be implemented by all functors used in the map-reduce operations.
 *
 * Since most uses of these functors involve critical operations, for performance reasons
 * virtual functions have been completely avoided. Although there is no actual syntactic enforcement
 * to inherit from this class, a compile-time assertion will be risen if this is not fulfilled.
 * Additionally a runtime assertion will rise if the bas class does not properly override the required methods.
 */
template <typename T>
struct MapReduceFunctorConcept {

  /**
   * \brief Perform the map-reduce functor operator. Version for unrolled template metaprogramming methods.
   *
   * Both \a Index and \a BlockSize are known at compile time in this version of the method.
   *
   * The functor should map the input arrays into a value and reduce the dimensionality by the amount
   * specified in \a BlockSize into the accumulator \a acc.
   * The first dimension index being reduced is defined by \a Index.
   *
   * \tparam index Index of the first dimension to reduce.
   * \tparam block_size Number of dimensions to reduce in this call.
   * \param acc Accumulator used to hold the reduced results.
   * \param a First input array.
   * \param b Second input array. May be \c NULL.
   * \param extra Extra arguments provided to the map-reduce function. May be \c NULL.
   * \return The accumulator \a acc updated with the operation.
   */
  template <unsigned int Index, unsigned int BlockSize, unsigned int D, typename AccumulatorType>
  inline AccumulatorType& operator () (AccumulatorType &acc, const T *a, const T *b, const void *extra) const {
    // Use the runtime-based method by default.
    return operator () (Index, BlockSize, D, acc, a, b, extra);
  }

  /**
   * \brief Perform the map-reduce functor operator. Version for loops.
   *
   * Both \a index and \a block_size are known at run time in this version of the method.
   *
   * The functor should map the input arrays into a value and reduce the dimensionality by the amount
   * specified in \a block_size into the accumulator \a acc.
   * The first dimension index being reduced is defined by \a index.
   *
   * \param index Index of the first dimension to reduce.
   * \param block_size Number of dimensions to reduce in this call.
   * \param acc Accumulator used to hold the reduced results.
   * \param a First input array.
   * \param b Second input array. May be \c NULL.
   * \param extra Extra arguments provided to the map-reduce function. May be \c NULL.
   * \return The accumulator \a acc updated with the operation.
   */
  template <unsigned int D, typename AccumulatorType>
  inline AccumulatorType& operator () (unsigned int index, unsigned int block_size, AccumulatorType &acc, const T *a, const T *b, const void *extra) const {
    KCHE_TREE_NEEDS_TO_BE_IMPLEMENTED();
    return acc;
  }
};

/**
 * \brief Base concept to be implemented by all functors used in the boundary checking operations.
 */
template <typename T>
struct BoundaryCheckFunctorConcept : public std::binary_function<T, T, bool> {
  /// Auxiliary type for optimized const references.
  typedef typename RParam<T>::Type ConstRef_T;

  inline bool operator () (ConstRef_T a, ConstRef_T b) const {
    KCHE_TREE_NEEDS_TO_BE_IMPLEMENTED();
    return false;
  }
};

/// 'Greater than' comparison functor for use with BoundedMapReduce.
template <typename T>
struct GreaterThanBoundaryFunctor : public BoundaryCheckFunctorConcept<T>, public std::greater<T> {
  using std::greater<T>::operator ();
};

} // namespace kche_tree

#endif
