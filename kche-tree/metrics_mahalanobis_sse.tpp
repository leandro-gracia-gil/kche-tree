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
 * \file metrics_mahalanobis_sse.tpp
 * \brief Template specializations for SSE optimizations applied to the Mahalanobis metric.
 * \author Leandro Graciá Gil
 */

// Include type traits and common SSE structures.
#include "traits.h"
#include "sse.h"

namespace kche_tree {

/// Map-reduce functor to calculate the difference between 2 SSE register vectors.
template <typename SSERegister>
struct DifferenceFunctorSSE : public SSEFunctor<SSERegister> {

  /// Return the difference of 2 SSE registers.
  template <typename Accumulator>
  inline void op(Accumulator &result, const SSERegister &a, const SSERegister &b) const {
    result.set_sub(a, b);
  }

  /// Loop-based version of the operation.
  template <unsigned int D>
  inline SSERegister*& operator () (unsigned int index, unsigned int block_size, SSERegister *&acc, const SSERegister *a, const SSERegister *b, const void *extra) const {
    KCHE_TREE_DCHECK(block_size == 1);
    op(acc[index], a[index], b[index]);
    return acc;
  }

  /// Unrolled compile-time version of the operation.
  template <unsigned int Index, unsigned int BlockSize, unsigned int D>
  inline SSERegister*& operator () (SSERegister *&acc, const SSERegister *a, const SSERegister *b, const void *extra) const {
    // Since this operation is purely SIMD parallel there is no benefit in processing multiple blocks together.
    KCHE_TREE_COMPILE_ASSERT(BlockSize == 1, "Expecting BlockSize == 1");
    return operator () <D>(Index, BlockSize, acc, a, b, extra);
  }
};

/// Map-reduce functor to calculate the dot product of 2 SSE register vectors.
template <typename SSERegister>
struct DotFunctorSSE : public SSEFunctor<SSERegister> {

  /// Accumulate the dot product of 2 consecutive pairs of SSE blocks. This allows to hide latencies caused by data dependencies.
  inline SSERegister& op2(SSERegister &acc, const SSERegister *a, const SSERegister *b, unsigned int i) const {
    SSERegister temp1, temp2;
    temp1.set_mult(a[i], b[i]);
    temp2.set_mult(a[i+1], b[i+1]);
    acc.set_add(acc, temp1);
    acc.set_add(acc, temp2);
    return acc;
  }

  /// Accumulate the dot product of 1 pair of SSE blocks.
  inline SSERegister& op1(SSERegister &acc, const SSERegister *a, const SSERegister *b, unsigned int i) const {
    SSERegister temp;
    temp.set_mult(a[i], b[i]);
    acc.set_add(acc, temp);
    return acc;
  }

  /// Loop-based version of the operation.
  template <unsigned int D>
  inline SSERegister& operator () (unsigned int index, unsigned int block_size, SSERegister &acc, const SSERegister *a, const SSERegister *b, const void *extra) const {
    KCHE_TREE_DCHECK(block_size == 1 || block_size == 2);
    if (block_size == 2)
      return op2(acc, a, b, index);
    else
      return op1(acc, a, b, index);
  }

  /// Unrolled compile-time version of the operation.
  template <unsigned int Index, unsigned int BlockSize, unsigned int D>
  inline SSERegister& operator () (SSERegister &acc, const SSERegister *a, const SSERegister *b, const void *extra) const {
    KCHE_TREE_COMPILE_ASSERT(BlockSize == 1 || BlockSize == 2, "Expecting BlockSize == 1 or 2");
    return operator () <D>(Index, BlockSize, acc, a, b, extra);
  }
};

/// Map-reduce functor to compute the accumulation of a^2 * b for 2 SSE register vectors.
template <typename SSERegister>
struct SquaredFirstDotFunctorSSE : public SSEFunctor<SSERegister> {

  /// Accumulate a^2 * b for 2 consecutive pairs of SSE blocks.
  inline SSERegister& op2(SSERegister &acc, const SSERegister *a, const SSERegister *b, unsigned int i) const {
    SSERegister temp1, temp2;
    temp1.set_mult(a[i], a[i]);
    temp2.set_mult(a[i+1], a[i+1]);
    temp1.set_mult(temp1, b[i]);
    temp2.set_mult(temp2, b[i+1]);
    acc.set_add(acc, temp1);
    acc.set_add(acc, temp2);
    return acc;
  }

  /// Accumulate a^2 * b for 1 pair of SSE blocks.
  inline SSERegister& op1(SSERegister &acc, const SSERegister *a, const SSERegister *b, unsigned int i) const {
    SSERegister temp;
    temp.set_mult(a[i], a[i]);
    temp.set_mult(temp, b[i]);
    acc.set_add(acc, temp);
    return acc;
  }

  /// Loop-based version of the operation.
  template <unsigned int D>
  inline SSERegister& operator () (unsigned int index, unsigned int block_size, SSERegister &acc, const SSERegister *a, const SSERegister *b, const void *extra) const {
    KCHE_TREE_DCHECK(block_size == 1 || block_size == 2);
    if (block_size == 2)
      return op2(acc, a, b, index);
    else
      return op1(acc, a, b, index);
  }

  /// Unrolled compile-time version of the operation.
  template <unsigned int Index, unsigned int BlockSize, unsigned int D>
  inline SSERegister& operator () (SSERegister &acc, const SSERegister *a, const SSERegister *b, const void *extra) const {
    KCHE_TREE_COMPILE_ASSERT(BlockSize == 1 || BlockSize == 2, "Expecting BlockSize = 1 or 2");
    return operator () <D>(Index, BlockSize, acc, a, b, extra);
  }
};

/// Map-reduce functor combining together the difference and squared first dot product operations for SSE.
template <typename SSERegister>
struct MahalanobisDiagonalFunctorSSE : public SSEFunctor<SSERegister> {

  /// Accumulate (a-b)^2 * c for a pair of values a, b and the diagonal value c stored in pairs of 2 consecutive SSE registers.
  inline SSERegister& op2(SSERegister &acc, const SSERegister *a, const SSERegister *b, const SSERegister *c, unsigned int i) const {
    SSERegister temp1, temp2;
    temp1.set_sub(a[i], b[i]);
    temp2.set_sub(a[i+1], b[i+1]);
    temp1.set_mult(temp1, temp1);
    temp2.set_mult(temp2, temp2);
    temp1.set_mult(temp1, c[i]);
    temp2.set_mult(temp2, c[i+1]);
    acc.set_add(acc, temp1);
    acc.set_add(acc, temp2);
    return acc;
  }

  /// Accumulate (a-b)^2 * c for a pair of values a, b and the diagonal value c stored in SSE registers.
  inline SSERegister& op1(SSERegister &acc, const SSERegister *a, const SSERegister *b, const SSERegister *c, unsigned int i) const {
    SSERegister temp;
    temp.set_sub(a[i], b[i]);
    temp.set_mult(temp, temp);
    temp.set_mult(temp, c[i]);
    acc.set_add(acc, temp);
    return acc;
  }

  /// Loop-based version of the operation. The \a extra param holds the matrix diagonal values.
  template <unsigned int D>
  inline SSERegister& operator () (unsigned int index, unsigned int block_size, SSERegister &acc, const SSERegister *a, const SSERegister *b, const void *extra) const {
    KCHE_TREE_DCHECK(block_size == 1 || block_size == 2);
    if (block_size == 2)
      return op2(acc, a, b, reinterpret_cast<const SSERegister*>(extra), index);
    else
      return op1(acc, a, b, reinterpret_cast<const SSERegister*>(extra), index);
  }

  /// Unrolled compile-time version of the operation. The \a extra param holds the matrix diagonal values.
  template <unsigned int Index, unsigned int BlockSize, unsigned int D>
  inline SSERegister& operator () (SSERegister &acc, const SSERegister *a, const SSERegister *b, const void *extra) const {
    KCHE_TREE_COMPILE_ASSERT(BlockSize == 1 || BlockSize == 2, "Expecting BlockSize == 1 or 2");
    return operator () <D>(Index, BlockSize, acc, a, b, extra);
  }
};

/**
 * \brief Map-reduce functor to calculate the full-matrix mahalanobis distance over a column of its inverse covariance matrix using SSE instructions.
 *
 * Since this method should be applied per column (therefore, per dimension) it operates as a scalar functor despite using SSE operations.
 */
template <typename T>
struct MahalanobisColumnFunctorSSE : public SSEFunctor<T> {

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
  inline SSERegister<T> & operator () (unsigned int index, unsigned int block_size, SSERegister<T> &acc, const T *cache, const T *not_used, const void *extra) const {
    KCHE_TREE_DCHECK(block_size == 1);
    KCHE_TREE_DCHECK(not_used == NULL);

    const unsigned int num_blocks_index = num_SSE_blocks<T>(index - 1);
    const SSERegister<T> *cache_sse = reinterpret_cast<const SSERegister<T> *>(cache);
    const SymmetricMatrix<T> *inv_covariance = reinterpret_cast<const SymmetricMatrix<T> *>(extra);

    const T *column_index = inv_covariance->column(index);
    const SSERegister<T> *column_index_sse = reinterpret_cast<const SSERegister<T> *>(column_index);

    // Calculate the dot product of (v1-v2) and the (i-1)-th column of the inverse covariance matrix.
    SSERegister<T> column_acc = SSERegister<T>::zero();
    non_unrolled_map_reduce<SSERegister<T>, NumSSEBlocks<T, D>::value>(DotFunctorSSE<SSERegister<T> >(), column_acc, cache_sse, column_index_sse, NULL, 0, num_blocks_index, 2);

    // Process the non-aligned elements using the scalar functors. Rotate the element accumulating the non-aligned results for better precision.
    non_unrolled_map_reduce<T, D>(DotFunctor<T>(), column_acc.data[index % SSETraits<T>::NumElements], cache, column_index, NULL, num_blocks_index * SSETraits<T>::NumElements, index);

    column_acc.set_mult(column_acc, SSERegister<T>::value(cache[index] * 2));
    return acc.set_add(acc, column_acc);
  }

  /**
   * \brief Unrolled compile-time version of the operation.
   *
   * \tparam Index Index of the column to process.
   * \tparam BlockSize Number of columns to process.
   * \tparam D Number of dimensions of the vectors.
   * \tparam Accumulator Type of the accumulator being used.
   *
   * \param acc Accumulator storing the partial result.
   * \param cache Precalculated vector of differences.
   * \param not_used Should be always \c NULL.
   * \param extra Pointer to the inverse covariance matrix object.
   * \return A reference to \a acc after being updated. Provided as syntactic sugar.
   */
  template <unsigned int Index, unsigned int BlockSize, unsigned int D>
  inline SSERegister<T> & operator () (SSERegister<T> &acc, const T *cache, const T *not_used, const void *extra) const {

    KCHE_TREE_COMPILE_ASSERT(BlockSize == 1, "Expecting BlockSize == 1");
    KCHE_TREE_DCHECK(not_used == NULL);

    const unsigned int num_blocks_index = NumSSEBlocks<T, Index - 1>::value;
    const SSERegister<T> *cache_sse = reinterpret_cast<const SSERegister<T> *>(cache);
    const SymmetricMatrix<T> *inv_covariance = reinterpret_cast<const SymmetricMatrix<T> *>(extra);

    const T *column_index = inv_covariance->column(Index);
    const SSERegister<T> *column_index_sse = reinterpret_cast<const SSERegister<T> *>(column_index);

    // Calculate the dot product of (v1-v2) and the (i-1)-th column of the inverse covariance matrix.
    SSERegister<T> column_acc = SSERegister<T>::zero();
    MapReduce<SSERegister<T>, num_blocks_index, 0, num_blocks_index, 2>::run(DotFunctorSSE<SSERegister<T> >(), column_acc, cache_sse, column_index_sse);
    MapReduce<T, Index, num_blocks_index * SSETraits<T>::NumElements>::run(DotFunctor<T>(), column_acc.data[Index % SSETraits<T>::NumElements], cache, column_index);

    column_acc.set_mult(column_acc, SSERegister<T>::value(cache[Index] * 2));
    return acc.set_add(acc, column_acc);
  }
};

/**
 * \brief SSE-optimized squared Mahalanobis distance calculator.
 *
 * \param v1 First feature vector.
 * \param v2 Second feature vector.
 * \param metric Mahalanobis metric object being used.
 * \return Squared Mahalanobis distance between the two vectors.
 */
template <typename T, const unsigned int D>
typename MahalanobisDistanceCalculatorSSE<T, D>::Distance MahalanobisDistanceCalculatorSSE<T, D>::distance(const Vector &v1, const Vector &v2, const Metric &metric) {

  // Access the input data as SSE registers. The required alignment should be automatically provided.
  const unsigned int num_blocks = NumSSEBlocks<T, D>::value;
  const SSERegister<T> *v1_sse = reinterpret_cast<const SSERegister<T> *>(v1.data());
  const SSERegister<T> *v2_sse = reinterpret_cast<const SSERegister<T> *>(v2.data());
  const SSERegister<T> *diagonal_sse = reinterpret_cast<const SSERegister<T> *>(metric.inverse_covariance().diagonal());

  // Prefetch the input data.
  v1_sse->prefetch();
  v2_sse->prefetch();

  // Initialize the accumulator.
  SSERegister<T> acc = SSERegister<T>::zero();

  // Don't precalculate the cache if the covariance matrix is diagonal.
  if (metric.has_diagonal_covariance()) {
    diagonal_sse->prefetch();
    MapReduce<SSERegister<T>, num_blocks>::run(MahalanobisDiagonalFunctorSSE<SSERegister<T> >(), acc, v1_sse, v2_sse, diagonal_sse);
    return acc.sum();
  }

  // Map the difference into a separate vector. No reduction operation is performed.
  typename MahalanobisMetric<T, D>::CacheVector cache;
  initSSEAlignmentGap(cache.mutable_data(), D);
  SSERegister<T> *cache_sse = reinterpret_cast<SSERegister<T> *>(cache.mutable_data());
  MapReduce<SSERegister<T>, num_blocks>::run(DifferenceFunctorSSE<SSERegister<T> >(), cache_sse, v1_sse, v2_sse);

  // Operate over the inverse covariance matrix diagonal.
  diagonal_sse->prefetch();
  MapReduce<SSERegister<T>, num_blocks>::run(SquaredFirstDotFunctorSSE<SSERegister<T> >(), acc, cache_sse, diagonal_sse);

  // Operate over the rest of the matrix.
  MapReduce<T, D, 1>::run(MahalanobisColumnFunctorSSE<T>(), acc, cache.data(), NULL, &metric.inverse_covariance());

  return acc.sum();
}

/**
 * \brief SSE-optimized squared Mahalanobis distance calculator with early leave when reaching an upper bound value.
 *
 * \param v1 First feature vector.
 * \param v2 Second feature vector.
 * \param metric Mahalanobis metric object being used.
 * \param upper_bound Upper boundary value used for early-out.
 * \return Squared Mahalanobis distance between the two vectors or the partial result if greater than \a upper_bound.
 */
template <typename T, const unsigned int D>
typename MahalanobisDistanceCalculatorSSE<T, D>::Distance MahalanobisDistanceCalculatorSSE<T, D>::distance(const Vector &v1, const Vector &v2, const Metric &metric, ConstRef_Distance upper_bound) {

  // Constant calculated empirically.
  const unsigned int D_acc = (unsigned int) (0.4f * D);

  // Access the input data as SSE registers. The required alignment should be automatically provided.
  const unsigned int num_blocks = NumSSEBlocks<T, D>::value;
  const unsigned int num_blocks_acc = NumSSEBlocks<T, D_acc>::value;
  const SSERegister<T> *v1_sse = reinterpret_cast<const SSERegister<T> *>(v1.data());
  const SSERegister<T> *v2_sse = reinterpret_cast<const SSERegister<T> *>(v2.data());
  const SSERegister<T> *diagonal_sse = reinterpret_cast<const SSERegister<T> *>(metric.inverse_covariance().diagonal());

  // Prefetch the input data.
  v1_sse->prefetch();
  v2_sse->prefetch();

  // Initialize the accumulator.
  SSERegister<T> acc = SSERegister<T>::zero();

  // Don't precalculate the cache if the covariance matrix is diagonal.
  if (metric.has_diagonal_covariance()) {
    // Check the details below to see why this operation is mathematically safe.
    diagonal_sse->prefetch();
    MapReduce<SSERegister<T>, num_blocks, 0, num_blocks_acc>::run(MahalanobisDiagonalFunctorSSE<SSERegister<T> >(), acc, v1_sse, v2_sse, diagonal_sse);
    BoundedMapReduce<3, SSERegister<T>, num_blocks, num_blocks_acc, num_blocks>::run(MahalanobisDiagonalFunctorSSE<SSERegister<T> >(), acc, GreaterThanBoundaryFunctorSSE<Distance>(), upper_bound, v1_sse, v2_sse, diagonal_sse);
    return acc.sum();
  }

  // Map the difference into a separate vector. No reduction operation is performed.
  typename MahalanobisMetric<T, D>::CacheVector cache;
  initSSEAlignmentGap(cache.mutable_data(), D);
  SSERegister<T> *cache_sse = reinterpret_cast<SSERegister<T> *>(cache.mutable_data());
  MapReduce<SSERegister<T>, num_blocks>::run(DifferenceFunctorSSE<SSERegister<T> >(), cache_sse, v1_sse, v2_sse);

  // Unfortunately this part of the calculation is not monotonically increasing and no early outs based on
  // incremental calculations are possible with this approach. BoundedMapReduce should not be used here.
  // For further mathematical details check the comment below.
  MapReduce<T, D, 1>::run(MahalanobisColumnFunctorSSE<T>(), acc, cache.data(), NULL, &metric.inverse_covariance());

  // The matrix is symmetric and assumed to be positive-definite. This comes from the assumption that it's the inverse of a covariance matrix,
  // which enforces the original covariance matrix to be symmetric positive-semidefinite, and the fact that its invert method fails
  // in case the matrix is not invertible (hence ensuring positive-definiteness). As a consequence, the matrix is also positive-definite
  // since it's the inverse of another positive-definite matrix and therefore, as a positive-definite Hermian matrix, all the elements
  // in its main diagonal will always be positive. This allows BoundedMapReduce to be safely used, as the distance function becames
  // monotonically increasing: a sum of squared differences and positive diagonal values.
  MapReduce<SSERegister<T>, num_blocks, 0, num_blocks_acc>::run(SquaredFirstDotFunctorSSE<SSERegister<T> >(), acc, cache_sse, diagonal_sse);
  BoundedMapReduce<3, SSERegister<T>, num_blocks, num_blocks_acc, num_blocks>::run(SquaredFirstDotFunctorSSE<SSERegister<T> >(), acc, GreaterThanBoundaryFunctorSSE<Distance>(), upper_bound, cache_sse, diagonal_sse);

  return acc.sum();
}

} // namespace kche_tree
