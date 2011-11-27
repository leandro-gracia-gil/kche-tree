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
 * \file metrics_euclidean_sse.tpp
 * \brief Template specializations for SSE optimizations applied to the Euclidean metric.
 * \author Leandro Graciá Gil
 */

// Include type traits and common SSE structures.
#include "traits.h"
#include "sse.h"

namespace kche_tree {

/// Map-reduce functor to calculate the dot product of the difference between 2 SSE register vectors.
template <typename SSERegisterType>
struct DifferenceDotFunctorSSE : public MapReduceFunctorConcept<SSERegisterType> {

  // Compile-time based version of the operation.
  template <unsigned int Index, unsigned int BlockSize, unsigned int D>
  inline SSERegisterType & operator () (SSERegisterType &acc, const SSERegisterType *a, const SSERegisterType *b, const void *extra) const {
    return operator () <D>(Index, BlockSize, acc, a, b, extra);
  }

  /// Runtime-based version of the operation.
  template <unsigned int D>
  inline SSERegisterType & operator () (unsigned int index, unsigned int block_size, SSERegisterType &acc, const SSERegisterType *a, const SSERegisterType *b, const void *extra) const {
    unsigned int i = index;

    // Process two blocks in parallel (reduces result waiting times).
    if (block_size == 2) {
      SSERegisterType temp1, temp2;
      temp1.set_sub(a[i], b[i]);
      temp2.set_sub(a[i+1], b[i+1]);
      temp1.set_mult(temp1, temp1);
      temp2.set_mult(temp2, temp2);
      acc.set_add(acc, temp1);
      acc.set_add(acc, temp2);
    }
    // Process one block in parallel.
    else {
      SSERegisterType temp;
      temp.set_sub(a[i], b[i]);
      temp.set_mult(temp, temp);
      acc.set_add(acc, temp);
    }

    return acc;
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
T EuclideanDistanceCalculatorSSE<T, D>::distance(const typename EuclideanMetric<T, D>::VectorType &v1, const typename EuclideanMetric<T, D>::VectorType &v2) {
  KCHE_TREE_COMPILE_ASSERT(IsPowerOfTwo<SSETraits<T>::NumElements>::value, "The number of elements in a SSE register must be a power of two.");

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
T EuclideanDistanceCalculatorSSE<T, D>::distance(const typename EuclideanMetric<T, D>::VectorType &v1, const typename EuclideanMetric<T, D>::VectorType &v2, const typename EuclideanMetric<T, D>::ConstRef_T upper_bound) {

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
  BoundedMapReduce<2, SSERegister<T>, num_blocks, num_blocks_acc, num_blocks, 2>::run(DifferenceDotFunctorSSE<SSERegister<T> >(), acc, GreaterThanBoundaryFunctorSSE<T>(), upper_bound, v1_sse, v2_sse);

  return acc.sum();
}

} // namespace kche_tree
