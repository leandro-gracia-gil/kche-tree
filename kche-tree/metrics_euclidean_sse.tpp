/***************************************************************************
 *   Copyright (C) 2011, 2012 by Leandro Graciá Gil                        *
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
 * \file metrics_euclidean_sse.tpp
 * \brief Template specializations for SSE optimizations applied to the Euclidean metric.
 * \author Leandro Graciá Gil
 */

// Include type traits and common SSE structures.
#include "traits.h"
#include "sse.h"

namespace kche_tree {

/// Map-reduce functor to calculate the dot product of the difference between 2 SSE register vectors.
template <typename SSERegister>
struct DifferenceDotFunctorSSE : public MapReduceFunctorConcept<SSERegister> {

  /// Accumulate the dot product of the difference of 2 consecutive pairs of SSE blocks. This allows to hide latencies caused by data dependencies.
  inline SSERegister& op2(SSERegister &acc, const SSERegister *a, const SSERegister *b, unsigned int i) const {
    SSERegister temp1, temp2;
    temp1.set_sub(a[i], b[i]);
    temp2.set_sub(a[i+1], b[i+1]);
    temp1.set_mult(temp1, temp1);
    temp2.set_mult(temp2, temp2);
    acc.set_add(acc, temp1);
    acc.set_add(acc, temp2);
    return acc;
  }

  /// Accumulate the dot product of the difference of 1 pair of SSE blocks.
  inline SSERegister& op1(SSERegister &acc, const SSERegister *a, const SSERegister *b, unsigned int i) const {
    SSERegister temp;
    temp.set_sub(a[i], b[i]);
    temp.set_mult(temp, temp);
    acc.set_add(acc, temp);
    return acc;
  }

  /// Runtime-based version of the operation.
  template <unsigned int D>
  inline SSERegister& operator () (unsigned int index, unsigned int block_size, SSERegister &acc, const SSERegister *a, const SSERegister *b, const void *extra) const {
    KCHE_TREE_DCHECK(block_size == 1 || block_size == 2);
    if (block_size == 2)
      return op2(acc, a, b, index);
    else
      return op1(acc, a, b, index);
  }

  // Compile-time based version of the operation.
  template <unsigned int Index, unsigned int BlockSize, unsigned int D>
  inline SSERegister& operator () (SSERegister &acc, const SSERegister *a, const SSERegister *b, const void *extra) const {
    KCHE_TREE_COMPILE_ASSERT(BlockSize == 1 || BlockSize == 2, "Expecting BlockSize == 1 or 2");
    return operator () <D>(Index, BlockSize, acc, a, b, extra);
  }
};

/**
 * \brief SSE-optimized squared Euclidean distance calculator.
 *
 * \param v1 First feature vector.
 * \param v2 Second feature vector.
 * \return Squared Euclidean distance between the two vectors.
 */
template <typename T, const unsigned int D>
typename EuclideanDistanceCalculatorSSE<T, D>::Distance EuclideanDistanceCalculatorSSE<T, D>::distance(const Vector &v1, const Vector &v2) {
  const unsigned int num_blocks = NumSSEBlocks<T, D>::value;
  const SSERegister<T> *v1_sse = reinterpret_cast<const SSERegister<T> *>(v1.data());
  const SSERegister<T> *v2_sse = reinterpret_cast<const SSERegister<T> *>(v2.data());

  v1_sse->prefetch();
  v2_sse->prefetch();

  SSERegister<T> acc = SSERegister<T>::zero();
  MapReduce<SSERegister<T>, num_blocks, 0, num_blocks, 2>::run(DifferenceDotFunctorSSE<SSERegister<T> >(), acc, v1_sse, v2_sse);
  return acc.sum();
}

/**
 * \brief SSE-optimized squared Euclidean distance calculator with early-out when reaching an upper bound value.
 *
 * \param v1 First feature vector.
 * \param v2 Second feature vector.
 * \param upper_bound Upper boundary value used for early-out.
 * \return Squared Euclidean distance between the two vectors or the partial result if greater than \a upper_bound.
 */
template <typename T, const unsigned int D>
typename EuclideanDistanceCalculatorSSE<T, D>::Distance EuclideanDistanceCalculatorSSE<T, D>::distance(const Vector &v1, const Vector &v2, ConstRef_Distance upper_bound) {

  const unsigned int num_blocks = NumSSEBlocks<T, D>::value;
  const SSERegister<T> *v1_sse = reinterpret_cast<const SSERegister<T> *>(v1.data());
  const SSERegister<T> *v2_sse = reinterpret_cast<const SSERegister<T> *>(v2.data());

  v1_sse->prefetch();
  v2_sse->prefetch();

  SSERegister<T> acc = SSERegister<T>::zero();

  // Process part of the vector sequentially and the rest with boundary checks.
  const unsigned int D_acc = (unsigned int) (0.2f * D);
  const unsigned int num_blocks_acc = NumSSEBlocks<T, D_acc>::value;
  MapReduce<SSERegister<T>, num_blocks_acc, 0, num_blocks_acc, 2>::run(DifferenceDotFunctorSSE<SSERegister<T> >(), acc, v1_sse, v2_sse);
  BoundedMapReduce<2, SSERegister<T>, num_blocks, num_blocks_acc, num_blocks, 2>::run(DifferenceDotFunctorSSE<SSERegister<T> >(), acc, GreaterThanBoundaryFunctorSSE<Distance>(), upper_bound, v1_sse, v2_sse);

  return acc.sum();
}

} // namespace kche_tree
