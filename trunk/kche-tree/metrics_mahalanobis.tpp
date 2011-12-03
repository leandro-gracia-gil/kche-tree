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
 * \file metrics_mahalanobis.tpp
 * \brief Template implementations for the Mahalanobis metric.
 * \author Leandro Graciá Gil
 */

// Include the map-reduce metaprograming templates and traits.
#include "map_reduce.h"
#include "traits.h"

namespace kche_tree {

/// Create a Mahalanobis metric object with the identity as its inverse covariance matrix.
template <typename T, const unsigned int D>
MahalanobisMetric<T, D>::MahalanobisMetric()
    : inv_covariance_(D, true),
      is_diagonal_(true) {
}

/**
 * \brief Create a Mahalanobis metric object and calculate its inverse covariance matrix from the provided data set.
 *
 * \param data_set Data set from which the inverse covariance matrix for the metric will be calculated.
 */
template <typename T, const unsigned int D>
MahalanobisMetric<T, D>::MahalanobisMetric(const DataSetType &data_set)
    : inv_covariance_(D, false) {
  set_inverse_covariance(data_set);
}

/**
 * \brief Calculate the inverse covariance matrix of the object from the data of a provided set.
 *
 * To perform this, the covariance matrix of the data in the set will be calculated and stored as a
 * symmetric matrix. Then, this matrix will be inverted using a specialized version of the LDL'
 * decomposition for symmetric matrices. The result is then stored as the inverse covariance matrix.
 *
 * \note If the resulting covariance matrix for the provided data set is not invertible then the contents
 * of the existing matrix won't be updated.
 *
 * \warning The provided data set should have at least 2 elements on it for this method to work.
 *
 * \param data_set Data set from which the inverse covariance matrix for the metric will be calculated.
 * \return \c true if succesfully set, \c false if the resulting covariance matrix is not invertible.
 */
template <typename T, const unsigned int D>
bool MahalanobisMetric<T, D>::set_inverse_covariance(const DataSetType &data_set) {

  // Calculate the set means.
  typedef typename Traits<T>::AccumulatorType AccumT;
  AccumT mean[D];
  for (unsigned int i=0; i<D; ++i)
    mean[i] = Traits<AccumT>::zero();

  unsigned int N = data_set.size();
  if (N <= 1)
    return false;

  for (unsigned int j=0; j<N; ++j)
    for (unsigned int i=0; i<D; ++i)
      mean[i] += data_set[j][i];

  float inv_N = 1.0f / N;
  for (unsigned int i=0; i<D; ++i)
    mean[i] *= inv_N;

  // Calculate the covariance matrix.
  SymmetricMatrix<T> new_inv_covariance(inv_covariance_.size());
  float inv_N1 = 1.0f / (N - 1);
  for (unsigned int j=0; j<D; ++j) {
    for (unsigned int i=0; i<=j; ++i) {

      AccumT acc = Traits<AccumT>::zero();
      for (unsigned int k=0; k<N; ++k) {
        T temp = data_set[k][i];
        temp -= mean[i];
        T temp2 = data_set[k][j];
        temp2 -= mean[j];
        temp *= temp2;
        acc += temp;
      }
      acc *= inv_N1;
      new_inv_covariance(j, i) = acc;
    }
  }

  // Invert it to get the inverse covariance matrix.
  if (!new_inv_covariance.invert())
    return false;

  // The matrix was invertible, copy the results.
  inv_covariance_ = new_inv_covariance;

  // Assume that the resulting matrix is not a diagonal one.
  is_diagonal_ = false;
  return true;
}

/**
 * \brief Set explicitly the values of the inverse covariance matrix.
 *
 * The values are assumed to be provided in row order with each row up to the main diagonal (covariance matrices are always symmetric).
 * The existing matrix contents won't be modified if the provied values are not valid.
 *
 * \note The matrix is assumed to have the properties of the inverse of a covariance matrix, in this case
 *       symmetric positive-definite, as inverting will fail for non-invertible positive-semidefinite ones.
 *       Consequently, since it's also symmetric all diagonal values must be positive.
 *
 * \param inverse_covariance Row-ordered array of D * (D + 1) / 2 values defining the new inverse covariance matrix.
 * \return \c true if the values are valid, \c false otherwise.
 */
template <typename T, const unsigned int D>
bool MahalanobisMetric<T, D>::set_inverse_covariance(const T *inverse_covariance) {

  // Ensure all diagonal values are positive.
  for (unsigned int j=0; j<D; ++j) {
    if (!(inverse_covariance(j, j) > Traits<T>::zero()))
      return false;
  }

  // Set the lower triangular values of the matrix (it's symmetric).
  unsigned int k = 0;
  for (unsigned int j=0; j<D; ++j) {
    for (unsigned int i=0; i<=j; ++i)
      inv_covariance_(j, i) = inverse_covariance[k++];
  }

  // Assume that the provided matrix is not a diagonal one.
  is_diagonal_ = false;
  return true;
}

/**
 * \brief Resets the current inverse covariance matrix to a diagonal matrix with the provided values.
 *
 * The values in \a diagonal should be a set of variances. This method will calculate their
 * multiplicative inverses and store them in the diagonals of the inverse covariance matrix.
 *
 * \note By the properties of the covariance matrices all the diagonal values should be positive.
 *       The existing matrix contents won't be modified if the provied values are not valid.
 *
 * \param diagonal Array of \link kche_tree::SymmetricMatrix::size size()\endlink diagonal values.
 * \return \c true if the values are valid, \c false otherwise.
 */
template <typename T, const unsigned int D>
bool MahalanobisMetric<T, D>::set_diagonal_covariance(const T *diagonal) {

  for (unsigned int j=0; j<D; ++j)
    if (!(diagonal[j] > Traits<T>::zero()))
      return false;

  for (unsigned int j=0; j<D; ++j) {
    for (unsigned int i=0; i<j; ++i)
      inv_covariance_(j, i) = Traits<T>::zero();
    inv_covariance_(j, j) = diagonal[j];
    Traits<T>::invert(inv_covariance_(j, j));
  }

  is_diagonal_ = true;
  return true;
}

/**
 * \brief Drop any non-diagonal values from the current inverse covariance matrix.
 * This will enable optimizations that are only available with diagonal covariance matrices.
 *
 * \note By the properties of the covariance matrices all the diagonal values should be positive.
 */
template <typename T, const unsigned int D>
void MahalanobisMetric<T, D>::force_diagonal_covariance() {
  for (unsigned int j=0; j<D; ++j) {
    for (unsigned int i=0; i<j; ++i)
      inv_covariance_(j, i) = Traits<T>::zero();
    KCHE_TREE_DCHECK(inv_covariance_(j, j) > Traits<T>::zero());
  }

  is_diagonal_ = true;
}

/// Map-reduce functor to calculate the difference between 2 values.
template <typename T>
struct DifferenceFunctor : public MapReduceFunctorConcept<T> {

  /// Auxiliary type for optimized const references.
  typedef typename RParam<T>::Type ConstRef_T;

  /// Compute the difference of 2 values.
  template <typename AccumulatorType>
  inline void op(AccumulatorType &result, ConstRef_T a, ConstRef_T b) const {
    result = a;
    result -= b;
  }

  /// Loop-based version of the operation.
  template <unsigned int D, typename AccumulatorType>
  inline AccumulatorType & operator () (unsigned int index, unsigned int block_size, AccumulatorType &acc, const T *a, const T *b, const void *extra) const {
    KCHE_TREE_DCHECK(block_size == 1);
    op(acc[index], a[index], b[index]);
    return acc;
  }

  /// Unrolled compile-time version of the operation.
  template <unsigned int Index, unsigned int BlockSize, unsigned int D, typename AccumulatorType>
  inline AccumulatorType & operator () (AccumulatorType &acc, const T *a, const T *b, const void *extra) const {
    KCHE_TREE_COMPILE_ASSERT(BlockSize == 1, "Expecting BlockSize = 1");
    op(acc[Index], a[Index], b[Index]);
    return acc;
  }
};

/// Accumulate the dot product of 2 values.
template <typename T>
struct DotFunctor : public MapReduceFunctorConcept<T> {

  /// Auxiliary type for optimized const references.
  typedef typename RParam<T>::Type ConstRef_T;

  /// Accumulate the dot product of 2 values.
  template <typename AccumulatorType>
  inline AccumulatorType & op(AccumulatorType &acc, ConstRef_T a, ConstRef_T b) const {
    T temp = a;
    temp *= b;
    return acc += temp;
  }

  /// Loop-based version of the operation.
  template <unsigned int D, typename AccumulatorType>
  inline AccumulatorType & operator () (unsigned int index, unsigned int block_size, AccumulatorType &acc, const T *a, const T *b, const void *extra) const {
    KCHE_TREE_DCHECK(block_size == 1);
    return op(acc, a[index], b[index]);
  }

  /// Unrolled compile-time version of the operation.
  template <unsigned int Index, unsigned int BlockSize, unsigned int D, typename AccumulatorType>
  inline AccumulatorType & operator () (AccumulatorType &acc, const T *a, const T *b, const void *extra) const {
    KCHE_TREE_COMPILE_ASSERT(BlockSize == 1, "Expecting BlockSize = 1");
    return op(acc, a[Index], b[Index]);
  }
};

/// Map-reduce functor to accumulate a^2 * b for a pair of values a, b.
template <typename T>
struct SquaredFirstDotFunctor : public MapReduceFunctorConcept<T> {

  /// Auxiliary type for optimized const references.
  typedef typename RParam<T>::Type ConstRef_T;

  // Accumulate a^2 * b for a pair of values a, b.
  template <typename AccumulatorType>
  inline AccumulatorType & op(AccumulatorType &acc, ConstRef_T a, ConstRef_T b) const {
    T temp = a;
    temp *= a;
    temp *= b;
    return acc += temp;
  }

  /// Loop-based version of the operation.
  template <unsigned int D, typename AccumulatorType>
  inline AccumulatorType & operator () (unsigned int index, unsigned int block_size, AccumulatorType &acc, const T *a, const T *b, const void *extra) const {
    KCHE_TREE_DCHECK(block_size == 1);
    return op(acc, a[index], b[index]);
  }

  /// Unrolled compile-time version of the operation.
  template <unsigned int Index, unsigned int BlockSize, unsigned int D, typename AccumulatorType>
  inline AccumulatorType & operator () (AccumulatorType &acc, const T *a, const T *b, const void *extra) const {
    KCHE_TREE_COMPILE_ASSERT(BlockSize == 1, "Expecting BlockSize = 1");
    return op(acc, a[Index], b[Index]);
  }
};

/// Map-reduce functor combining together the difference and squared first dot product operations.
template <typename T>
struct MahalanobisDiagonalFunctor : public MapReduceFunctorConcept<T> {

  /// Auxiliary type for optimized const references.
  typedef typename RParam<T>::Type ConstRef_T;

  /// Accumulate (a-b)^2 * c for a pair of values a, b and the diagonal value c.
  template <typename AccumulatorType>
  inline AccumulatorType & op(AccumulatorType &acc, ConstRef_T a, ConstRef_T b, ConstRef_T c) const {
    T temp = a;
    temp -= b;
    temp *= temp;
    temp *= c;
    return acc += temp;
  }

  /// Loop-based version of the operation. The \a extra param holds the matrix diagonal values.
  template <unsigned int D, typename AccumulatorType>
  inline AccumulatorType & operator () (unsigned int index, unsigned int block_size, AccumulatorType &acc, const T *a, const T *b, const void *extra) const {
    KCHE_TREE_DCHECK(block_size == 1);
    return op(acc, a[index], b[index], reinterpret_cast<const T *>(extra)[index]);
  }

  /// Unrolled compile-time version of the operation. The \a extra param holds the matrix diagonal values.
  template <unsigned int Index, unsigned int BlockSize, unsigned int D, typename AccumulatorType>
  inline AccumulatorType & operator () (AccumulatorType &acc, const T *a, const T *b, const void *extra) const {
    KCHE_TREE_COMPILE_ASSERT(BlockSize == 1, "Expecting BlockSize = 1");
    return op(acc, a[Index], b[Index], reinterpret_cast<const T *>(extra)[Index]);
  }
};

/// Map-reduce functor to calculate the contribution of a column of the inverse covariance matrix to the mahalanobis distance.
template <typename T>
struct MahalanobisColumnFunctor : public MapReduceFunctorConcept<T> {

  /**
   * \brief Loop-based version of the operation.
   *
   * \param index Index of the column to process.
   * \param block_size Number of columns to process.
   * \param acc Accumulator storing the partial result.
   * \param cache Precalculated vector of differences.
   * \param not_used Should be always \c NULL.
   * \param extra Pointer to the inverse covariance matrix object.
   * \return A reference to \a acc after being updated. Provided as syntactic sugar.
   */
  template <unsigned int D>
  inline T & operator () (unsigned int index, unsigned int block_size, T &acc, const T *cache, const T *not_used, const void *extra) const {
    KCHE_TREE_DCHECK(block_size == 1);
    KCHE_TREE_DCHECK(not_used == NULL);

    typedef typename Traits<T>::AccumulatorType AccumT;
    AccumT column_acc = Traits<AccumT>::zero();

    // Since index is a runtime value we need to use the loop-based version of map reduce.
    const SymmetricMatrix<T> *inv_covariance = reinterpret_cast<const SymmetricMatrix<T> *>(extra);
    non_unrolled_map_reduce<T, D>(DotFunctor<T>(), column_acc, cache, inv_covariance->column(index), NULL, 0, index);

    T aux = cache[index];
    aux += cache[index];
    column_acc *= aux;
    return acc += column_acc;
  }

  /**
   * \brief Unrolled compile-time version of the operation.
   *
   * \tparam Index Index of the column to process.
   * \tparam BlockSize Number of columns to process.
   * \tparam D Number of dimensions of the vectors.
   * \tparam AccumulatorType Type of the accumulator being used.
   *
   * \param acc Accumulator storing the partial result.
   * \param cache Precalculated vector of differences.
   * \param not_used Should be always \c NULL.
   * \param extra Pointer to the inverse covariance matrix object.
   * \return A reference to \a acc after being updated. Provided as syntactic sugar.
   */
  template <unsigned int Index, unsigned int BlockSize, unsigned int D, typename AccumulatorType>
  inline AccumulatorType & operator () (AccumulatorType &acc, const T *cache, const T *not_used, const void *extra) const {
    KCHE_TREE_COMPILE_ASSERT(BlockSize == 1, "Expecting BlockSize = 1");
    KCHE_TREE_DCHECK(not_used == NULL);

    typedef typename Traits<T>::AccumulatorType AccumT;
    AccumT column_acc = Traits<AccumT>::zero();

    // Calculate the dot product of (v1-v2) and the (i-1)-th column of the inverse covariance matrix.
    const SymmetricMatrix<T> *inv_covariance = reinterpret_cast<const SymmetricMatrix<T> *>(extra);
    MapReduce<T, Index>::run(DotFunctor<T>(), column_acc, cache, inv_covariance->column(Index));

    T aux = cache[Index];
    aux += cache[Index];
    column_acc *= aux;
    return acc += column_acc;
  }
};

/// Provides functions to calculate the Mahalanobis distance of two \a D dimensional vectors of type \a T.
template <typename T, const unsigned int D>
struct MahalanobisDistanceCalculator {
  static inline T distance(const typename MahalanobisMetric<T, D>::VectorType &v1, const typename MahalanobisMetric<T, D>::VectorType &v2,
      const MahalanobisMetric<T, D> &metric);
  static inline T distance(const typename MahalanobisMetric<T, D>::VectorType &v1, const typename MahalanobisMetric<T, D>::VectorType &v2,
      const MahalanobisMetric<T, D> &metric, typename MahalanobisMetric<T, D>::ConstRef_T upper_bound);
};

/**
 * \brief Provides functions to calculate the Mahalanobis distance of two \a D dimensional vectors of type \a T using SSE-optimizations if enabled.
 * Otherwise it defauls to the standard behaviour.
 */
template <typename T, const unsigned int D>
struct MahalanobisDistanceCalculatorSSE : MahalanobisDistanceCalculator<T, D> {
  static inline T distance(const typename MahalanobisMetric<T, D>::VectorType &v1, const typename MahalanobisMetric<T, D>::VectorType &v2,
      const MahalanobisMetric<T, D> &metric);
  static inline T distance(const typename MahalanobisMetric<T, D>::VectorType &v1, const typename MahalanobisMetric<T, D>::VectorType &v2,
      const MahalanobisMetric<T, D> &metric, typename MahalanobisMetric<T, D>::ConstRef_T upper_bound);
};

/**
 * \brief Generic squared Mahalanobis distance operator for two D-dimensional feature vectors.
 *
 * \param v1 First feature vector.
 * \param v2 Second feature vector.
 * \return Squared Mahalanobis distance between the two vectors.
 */
template <typename T, const unsigned int D>
T MahalanobisMetric<T, D>::operator () (const VectorType &v1, const VectorType &v2) const {

  // Delegate the distance calculation depending on the SSE optimization settings.
  typedef typename TypeBranch<Settings::enable_sse, MahalanobisDistanceCalculatorSSE<T, D>, MahalanobisDistanceCalculator<T, D> >::Result DistanceCalculator;
  return DistanceCalculator::distance(v1, v2, *this);
}

/**
 * \brief Generic squared Mahalanobis distance operator for two D-dimensional feature vectors.
 * Special version with early-out in case an upper bound value is reached.
 *
 * \param v1 Frist feature vector.
 * \param v2 Second feature vector.
 * \param upper_bound Upper bound for the distance. Will return immediatly if reached.
 * \return Squared Mahalanobis distance between the two vectors or a partial result greater than \a upper.
 */
template <typename T, const unsigned int D>
T MahalanobisMetric<T, D>::operator () (const VectorType &v1, const VectorType &v2, ConstRef_T upper_bound) const {

  // Delegate the distance calculation depending on the SSE optimization settings.
  typedef typename TypeBranch<Settings::enable_sse, MahalanobisDistanceCalculatorSSE<T, D>, MahalanobisDistanceCalculator<T, D> >::Result DistanceCalculator;
  return DistanceCalculator::distance(v1, v2, *this, upper_bound);
}

/**
 * \brief Generic squared Mahalanobis distance calculator.
 *
 * \param v1 First feature vector.
 * \param v2 Second feature vector.
 * \param metric Mahalanobis metric object being used.
 * \return Squared Mahalanobis distance between the two vectors.
 */
template <typename T, const unsigned int D>
T MahalanobisDistanceCalculator<T, D>::distance(const typename MahalanobisMetric<T, D>::VectorType &v1, const typename MahalanobisMetric<T, D>::VectorType &v2, const MahalanobisMetric<T, D> &metric) {

  // Initialize the result accumulator.
  typedef typename Traits<T>::AccumulatorType AccumT;
  AccumT acc = Traits<AccumT>::zero();

  // Don't precalculate the cache if the covariance matrix is diagonal.
  if (metric.has_diagonal_covariance()) {
      MapReduce<T, D>::run(MahalanobisDiagonalFunctor<T>(), acc, v1.data(), v2.data(), metric.inverse_covariance().diagonal());
      return acc;
  }

  // Map the difference into a separate vector. No reduction operation is performed.
  typename MahalanobisMetric<T, D>::CacheVectorType cache;
  T *cache_data = cache.mutable_data();
  MapReduce<T, D>::run(DifferenceFunctor<T>(), cache_data, v1.data(), v2.data());

  // Operate over the inverse covariance matrix diagonal.
  MapReduce<T, D>::run(SquaredFirstDotFunctor<T>(), acc, cache.data(), metric.inverse_covariance().diagonal());

  // Operate over the rest of the matrix.
  MapReduce<T, D, 1>::run(MahalanobisColumnFunctor<T>(), acc, cache.data(), NULL, &metric.inverse_covariance());

  return acc;
}

/**
 * \brief Generic squared Mahalanobis distance calculator with early-out when reaching an upper bound value.
 *
 * \param v1 First feature vector.
 * \param v2 Second feature vector.
 * \param metric Mahalanobis metric object being used.
 * \param upper_bound Upper boundary value used for early-out.
 * \return Squared Mahalanobis distance between the two vectors or the partial result if greater than \a upper_bound.
 */
template <typename T, const unsigned int D>
T MahalanobisDistanceCalculator<T, D>::distance(const typename MahalanobisMetric<T, D>::VectorType &v1, const typename MahalanobisMetric<T, D>::VectorType &v2, const MahalanobisMetric<T, D> &metric, typename MahalanobisMetric<T, D>::ConstRef_T upper_bound) {

  // Constant calculated empirically.
  const unsigned int D_acc = (unsigned int) (0.4f * D);

  // Initialize the result accumulator.
  typedef typename Traits<T>::AccumulatorType AccumT;
  AccumT acc = Traits<AccumT>::zero();

  // Don't precalculate the cache if the covariance matrix is diagonal.
  if (metric.has_diagonal_covariance()) {
    // Check the details below to see why this operation is mathematically safe.
    MapReduce<T, D, 0, D_acc>::run(MahalanobisDiagonalFunctor<T>(), acc, v1.data(), v2.data(), metric.inverse_covariance().diagonal());
    BoundedMapReduce<3, T, D, D_acc, D>::run(MahalanobisDiagonalFunctor<T>(), acc, GreaterThanBoundaryFunctor<T>(), upper_bound, v1.data(), v2.data(), metric.inverse_covariance().diagonal());
    return acc;
  }

  // Map the difference into a separate vector. No reduction operation is performed.
  typename MahalanobisMetric<T, D>::CacheVectorType cache;
  T *cache_data = cache.mutable_data();
  MapReduce<T, D>::run(DifferenceFunctor<T>(), cache_data, v1.data(), v2.data());

  // Unfortunately this part of the calculation is not monotonically increasing and no early outs based on
  // incremental calculations are possible with this approach. BoundedMapReduce should not be used here.
  // For further mathematical details check the comment below.
  MapReduce<T, D, 1>::run(MahalanobisColumnFunctor<T>(), acc, cache.data(), NULL, &metric.inverse_covariance());

  // The matrix is symmetric and assumed to be positive-definite. This comes from the assumption that it's the inverse of a covariance matrix,
  // which enforces the original covariance matrix to be symmetric positive-semidefinite, and the fact that its invert method fails
  // in case the matrix is not invertible (hence ensuring positive-definiteness). As a consequence, the matrix is also positive-definite
  // since it's the inverse of another positive-definite matrix and therefore, as a positive-definite Hermian matrix, all the elements
  // in its main diagonal will always be positive. This allows BoundedMapReduce to be safely used, as the distance function becomes
  // monotonically increasing: a sum of squared differences and positive diagonal values.
  MapReduce<T, D, 0, D_acc>::run(SquaredFirstDotFunctor<T>(), acc, cache.data(), metric.inverse_covariance().diagonal());
  BoundedMapReduce<3, T, D, D_acc, D>::run(SquaredFirstDotFunctor<T>(), acc, GreaterThanBoundaryFunctor<T>(), upper_bound, cache.data(), metric.inverse_covariance().diagonal());

  return acc;
}

} // namespace kche_tree

// Include the SSE specializations if enabled.
#if KCHE_TREE_ENABLE_SSE
#include "metrics_mahalanobis_sse.tpp"
#endif
