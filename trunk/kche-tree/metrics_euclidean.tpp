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
 * \file metrics_euclidean.tpp
 * \brief Template implementations for the Euclidean metric.
 * \author Leandro Graciá Gil
 */

// Include the map-reduce metaprograming templates and traits.
#include "map_reduce.h"
#include "traits.h"

namespace kche_tree {

// Forward declarations.
template <typename T, bool isFundamental = IsFundamental<T>::value>
struct DifferenceDotFunctor;

/// Provides functions to calculate the Euclidean distance of two \a D dimensional vectors of type \a T.
template <typename T, const unsigned int D>
struct EuclideanDistanceCalculator {
  static inline T distance(const typename EuclideanMetric<T, D>::VectorType &v1, const typename EuclideanMetric<T, D>::VectorType &v2);
  static inline T distance(const typename EuclideanMetric<T, D>::VectorType &v1, const typename EuclideanMetric<T, D>::VectorType &v2, typename EuclideanMetric<T, D>::ConstRef_T upper_bound);
};

/**
 * \brief Provides functions to calculate the Euclidean distance of two \a D dimensional vectors of type \a T using SSE-optimizations if enabled.
 * Otherwise it defauls to the standard behaviour.
 */
template <typename T, const unsigned int D>
struct EuclideanDistanceCalculatorSSE : EuclideanDistanceCalculator<T, D> {
  static inline T distance(const typename EuclideanMetric<T, D>::VectorType &v1, const typename EuclideanMetric<T, D>::VectorType &v2);
  static inline T distance(const typename EuclideanMetric<T, D>::VectorType &v1, const typename EuclideanMetric<T, D>::VectorType &v2, typename EuclideanMetric<T, D>::ConstRef_T upper_bound);
};

/// Map-reduce functor to calculate the dot functor of the difference between 2 fundamental values.
template <typename T>
struct DifferenceDotFunctor<T, true> : public MapReduceFunctorConcept<T> {

  /// Loop-based version of the operation.
  template <unsigned int D, typename AccumulatorType>
  inline AccumulatorType& operator () (unsigned int index, unsigned int block_size, AccumulatorType &acc, const T *a, const T *b, const void *extra) const {
    for (unsigned int k=0, i=index; k<block_size; ++k, ++i)
      acc += (a[i] - b[i]) * (a[i] - b[i]);
    return acc;
  }

  /// 'Unrolled' compile-time version of the operation.
  template <unsigned int Index, unsigned int BlockSize, unsigned int D, typename AccumulatorType>
  inline AccumulatorType& operator () (AccumulatorType &acc, const T *a, const T *b, const void *extra) const {
    KCHE_TREE_COMPILE_ASSERT(BlockSize == 1, "Expecting BlockSize = 1");
    return operator () <D>(Index, BlockSize, acc, a, b, extra);
  }
};

/**
 * \brief Map-reduce functor to calculate the dot functor of the difference between 2 non-fundamental values.
 *
 * Makes use of the =, -= and *= operators for the \a T type and the += operator for the accumulator type.
 */
template <typename T>
struct DifferenceDotFunctor<T, false> : public MapReduceFunctorConcept<T> {

  /// Loop-based version of the operation.
  template <unsigned int D, typename AccumulatorType>
  inline AccumulatorType& operator () (unsigned int index, unsigned int block_size, AccumulatorType &acc, const T *a, const T *b, const void *extra) const {
    for (unsigned int k=0, i=index; k<block_size; ++k, ++i) {
      T temp = a[i];
      temp -= b[i];
      temp *= temp;
      acc += temp;
    }
    return acc;
  }

  /// 'Unrolled' compile-time version of the operation.
  template <unsigned int Index, unsigned int BlockSize, unsigned int D, typename AccumulatorType>
  inline AccumulatorType& operator () (AccumulatorType &acc, const T *a, const T *b, const void *extra) const {
    return operator () <D>(Index, BlockSize, acc, a, b, extra);
  }
};

/**
 * \brief Generic squared euclidean distance operator for two D-dimensional feature vectors.
 *
 * \param v1 First feature vector.
 * \param v2 Second feature vector.
 * \return Euclidean squared distance between the two vectors.
 */
template <typename T, const unsigned int D>
T EuclideanMetric<T, D>::operator () (const VectorType &v1, const VectorType &v2) const {

  // Delegate the distance calculation depending on the SSE optimization settings.
  typedef typename TypeBranch<Settings::enable_sse, EuclideanDistanceCalculatorSSE<T, D>, EuclideanDistanceCalculator<T, D> >::Result DistanceCalculator;
  return DistanceCalculator::distance(v1, v2);
}

/**
 * \brief Generic squared euclidean distance operator for two D-dimensional feature vectors.
 * Special version with early-out in case an upper bound value is reached.
 *
 * \param v1 Frist feature vector.
 * \param v2 Second feature vector.
 * \param upper_bound Upper bound for the distance. Will return immediatly if reached.
 * \return Euclidean squared distance between the two vectors or a partial result greater than \a upper.
 */
template <typename T, const unsigned int D>
T EuclideanMetric<T, D>::operator () (const VectorType &v1, const VectorType &v2, ConstRef_T upper_bound) const {

  // Delegate the distance calculation depending on the SSE optimization settings.
  typedef typename TypeBranch<Settings::enable_sse, EuclideanDistanceCalculatorSSE<T, D>, EuclideanDistanceCalculator<T, D> >::Result DistanceCalculator;
  return DistanceCalculator::distance(v1, v2, upper_bound);
}

/**
 * \brief Generic squared Euclidean distance calculator.
 *
 * \param v1 First feature vector.
 * \param v2 Second feature vector.
 * \return Squared Euclidean distance between the two vectors.
 */
template <typename T, const unsigned int D>
T EuclideanDistanceCalculator<T, D>::distance(const typename EuclideanMetric<T, D>::VectorType &v1, const typename EuclideanMetric<T, D>::VectorType &v2) {

  // Standard squared distance between two D-dimensional vectors.
  typedef typename Traits<T>::AccumulatorType AccumT;
  AccumT acc = Traits<AccumT>::zero();
  MapReduce<T, D>::run(DifferenceDotFunctor<T>(), acc, v1.data(), v2.data());
  return acc;
}

/**
 * \brief Generic squared Euclidean distance calculator with early-out when reaching an upper bound value.
 *
 * \param v1 First feature vector.
 * \param v2 Second feature vector.
 * \param upper_bound Upper boundary value used for early-out.
 * \return Squared Euclidean distance between the two vectors or the partial result if greater than \a upper_bound.
 */
template <typename T, const unsigned int D>
T EuclideanDistanceCalculator<T, D>::distance(const typename EuclideanMetric<T, D>::VectorType &v1, const typename EuclideanMetric<T, D>::VectorType &v2, typename EuclideanMetric<T, D>::ConstRef_T upper_bound) {

  // Constant calculated empirically.
  const unsigned int D_acc = (unsigned int) (0.4f * D);

  // Accumulate the first D_acc dimensions without any kind of check.
  typename Traits<T>::AccumulatorType acc = Traits<T>::zero();
  MapReduce<T, D, 0, D_acc>::run(DifferenceDotFunctor<T>(), acc, v1.data(), v2.data());

  // Calculate the remaining dimensions using an upper bound, and checking it every 4 dimensions.
  // The template metaprogramming makes sure this interval is performed without actually checking any index or iterator at runtime.
  // Has been tested to be faster than a loop with the difference being more acute with greater D values.
  BoundedMapReduce<4, T, D, D_acc>::run(DifferenceDotFunctor<T>(), acc, GreaterThanBoundaryFunctor<T>(), upper_bound, v1.data(), v2.data());
  return acc;
}

} // namespace kche_tree

// Include the SSE specializations if enabled.
#if KCHE_TREE_ENABLE_SSE
#include "metrics_euclidean_sse.tpp"
#endif
