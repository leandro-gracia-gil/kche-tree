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
 * \file sse.h
 * \brief Utility templates and types for SSE operations.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_SSE_H_
#define _KCHE_TREE_SSE_H_

#if KCHE_TREE_ENABLE_SSE
// Functions for the SSE instruction set.
#include <xmmintrin.h>
#endif

#include "map_reduce.h"
#include "utils.h"

namespace kche_tree {

// Definitions for disabled or non-supported SSE.
#if !(KCHE_TREE_ENABLE_SSE)

// Default SSE alignment correction.
#define KCHE_TREE_SSE_COMPILE_ALIGN(type, size) (size)
#define KCHE_TREE_SSE_RUNTIME_ALIGN(type, size) (size)

template <typename T>
void initSSEAlignmentGap(T *array, unsigned int size) {}

// From this point only if SSE is both supported and enabled.
#else

// Macros used for specifying alignment.
#if defined(__GNUC__)
#define KCHE_TREE_ALIGNED(x) __attribute__((aligned(x)))
#elif defined(_MFC_VER)
#define KCHE_TREE_ALIGNED(x) __declspec(align(x))
#else
#error "Data alignment directive required, but not defined for your compiler. Check sse.h."
#endif

/// SSE-specific traits. Designed to be only valid for specialized types.
template <typename T>
struct SSETraits {
  /// Type of the SSE register to use.
  typedef void RegisterType;

  /// Auxiliary type for optimized const references.
  typedef typename RParam<T>::Type ConstRef_T;

  /// Number of elements contained in the SSE register.
  static const unsigned int NumElements = 0;

  /// Return a register initialized to zero.
  static inline RegisterType zero() {}

  /// Return a register initialized to the provided value.
  static inline RegisterType value(ConstRef_T value) {}
};

/// SSE information for the float type.
template <>
struct SSETraits<float> {
  /// Define __m128 as the single precision floating point SSE register.
  typedef __m128 RegisterType;

  /// SSE __m128 registers contain 4 floating point values.
  static const unsigned int NumElements = 4;

  /// Return a register initialized to zero.
  static inline RegisterType zero() {
    return _mm_setzero_ps();
  }

  /// Return a register with one element initialized to the provided value.
  static inline RegisterType value(float value) {
    return _mm_set1_ps(value);
  }

  static inline RegisterType add(const RegisterType &a, const RegisterType &b) {
    return _mm_add_ps(a, b);
  }

  static inline RegisterType sub(const RegisterType &a, const RegisterType &b) {
    return _mm_sub_ps(a, b);
  }

  static inline RegisterType mult(const RegisterType &a, const RegisterType &b) {
    return _mm_mul_ps(a, b);
  }
};

/**
 * \brief Generic SSE register definition.
 *
 * Designed to raise compile assertions on non-allowed types.
 * All type-specific information should go in the SSE traits.
 */
template <typename T>
union SSERegister {

  // Ensure the provided type is one supported for SSE.
  KCHE_TREE_COMPILE_ASSERT((!is_same<typename SSETraits<T>::RegisterType, void>::value || SSETraits<T>::NumElements == 0), "Type not supported for SSE calculations.");

  // Ensure the type T is not configured to use any custom accumulator types.
  KCHE_TREE_COMPILE_ASSERT((is_same<typename Traits<T>::AccumulatorType, T>::value),
      "Accumulation types different than the type itself are not supported by the SSE versions");

  /// Type contained by the register.
  typedef T ElementType;

  /// SSE register to be used in the calculations.
  typename SSETraits<T>::RegisterType reg;

  /// Auxiliary type for optimized const references.
  typedef typename RParam<T>::Type ConstRef_T;

  /// Array of elements of type \a T to access the contents of the SSE register.
  T data[SSETraits<T>::NumElements];

  /// Sum the individual contents of the SSE register.
  T sum() const {
    T sum = data[0];
    for (unsigned int i=1; i<SSETraits<T>::NumElements; ++i)
      sum += data[i];
    return sum;
  }

  /// Prefetch the object address into cache memory.
  void prefetch() const {
    _mm_prefetch(this, _MM_HINT_NTA);
  }

  /// Return a SSERegister object initialized to zero.
  static inline SSERegister zero() {
    SSERegister z = { SSETraits<T>::zero() };
    return z;
  }

  /// Return a register with one element initialized to the provided value.
  static inline SSERegister value(ConstRef_T value) {
    SSERegister v = { SSETraits<T>::value(value) };
    return v;
  }

  /// Add two SSE registers and set the result in the local object.
  inline SSERegister &set_add(const SSERegister &a, const SSERegister &b) {
    reg = SSETraits<T>::add(a.reg, b.reg);
    return *this;
  }

  /// Substract two SSE registers and set the result in the local object.
  inline SSERegister &set_sub(const SSERegister &a, const SSERegister &b) {
    reg = SSETraits<T>::sub(a.reg, b.reg);
    return *this;
  }

  /// Multiply two SSE registers and set the result in the local object.
  inline SSERegister &set_mult(const SSERegister &a, const SSERegister &b) {
    reg = SSETraits<T>::mult(a.reg, b.reg);
    return *this;
  }
};

/**
 * \brief Base class for SSE-specific functors.
 *
 * Designed to be implemented by means of type specializations.
 * Raises runtime assertions in case no such specialization exists.
 */
template <typename T>
struct SSEFunctor : public MapReduceFunctorConcept<T> {

  /// Implemented by specializations.
  template <unsigned int Index, unsigned int BlockSize, typename AccumulatorType>
  inline AccumulatorType& operator () (AccumulatorType &acc, const T *a, const T *b, void *extras) const {
    KCHE_TREE_NOT_REACHED();
    return acc;
  }

  /// Implemented by specializations.
  template <typename AccumulatorType>
  inline AccumulatorType& operator () (unsigned int index, unsigned int block_size, AccumulatorType &acc, const T *a, const T *b, void *extras) const {
    KCHE_TREE_NOT_REACHED();
    return acc;
  }
};

/// Generic SSE 'greater than' comparison functor for use with BoundedMapReduce.
template <typename T>
struct GreaterThanBoundaryFunctorSSE : public BoundaryCheckFunctorConcept<T> {
  /// Auxiliary type for optimized const references.
  typedef typename RParam<T>::Type ConstRef_T;

  bool operator () (const SSERegister<T> &acc, ConstRef_T boundary) const {
    return acc.sum() > boundary;
  }
};

/// Calculates the number of SSE blocks required to cover a \a D dimensional array of type \a T.
template <typename T, unsigned int D>
struct NumSSEBlocks {
  /// Desired number of blocks. Assumes proper array size (SSE alignment macros were used) and alignment.
  enum { value = NextMultipleOfPOT<SSETraits<T>::NumElements, D>::value / SSETraits<T>::NumElements };
};

/// Calculates in runtime the number of SSE blocks required to cover a \a D dimensional array of type \a T.
template <typename T>
unsigned int num_SSE_blocks(unsigned int D) {
  return next_multiple_of_pot(SSETraits<T>::NumElements, D) / SSETraits<T>::NumElements;
}

/// Define an alignment of multiples of the SSE register elements for the number of dimensions (compile-time version).
#undef KCHE_TREE_SSE_COMPILE_ALIGN
#define KCHE_TREE_SSE_COMPILE_ALIGN(type, size) NextMultipleOfPOT<SSETraits<type>::NumElements, size>::value

/// Define an alignment of multiples of the SSE register elements for the number of dimensions (runtime version).
#undef KCHE_TREE_SSE_RUNTIME_ALIGN
#define KCHE_TREE_SSE_RUNTIME_ALIGN(type, size) (((size) + (SSETraits<type>::NumElements - 1)) & ~(SSETraits<type>::NumElements - 1))

/**
 * \brief Initialize the alignment gap of an array with zero values.
 *
 * \param array Array to initialize.
 * \param size Original size of the array (unaligned).
 */
template <typename T>
void initSSEAlignmentGap(T *array, unsigned int size) {
  const unsigned int aligned_size = KCHE_TREE_SSE_RUNTIME_ALIGN(T, size);
  for (unsigned int i=size; i<aligned_size; ++i)
    array[i] = Traits<T>::zero();
}

#endif // KCHE_TREE_ENABLE_SSE

} // namespace kche_tree

// Include SSE2 specifics.
#include "sse2.h"

#endif
