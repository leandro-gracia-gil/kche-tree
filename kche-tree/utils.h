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
 * \file utils.h
 * \brief Utility templates for template metaprogramming and others.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_UTILS_H_
#define _KCHE_TREE_UTILS_H_

#include "cpp1x.h"

namespace kche_tree {

// Macro definitions for concept checking.
#define KCHE_TREE_NEEDS_TO_BE_IMPLEMENTED() KCHE_TREE_NOT_REACHED()

#define KCHE_TREE_CHECK_CONCEPT(type, base) \
    KCHE_TREE_COMPILE_ASSERT((IsBaseOf<base, type>::value), "Concept error. " # type " should implement " # base ".")

// Utility classes.

/**
 * \brief Base class for non-copyable objects.
 *
 * Leaves undefined or deletes (if C++1x is enabled) the copy constructor and the assignment operator.
 */
class NonCopyable {
#ifdef KCHE_TREE_DISABLE_CPP1X
  NonCopyable(const NonCopyable &);
  NonCopyable &operator = (const NonCopyable &);
#else
  NonCopyable(const NonCopyable &) = delete;
  NonCopyable &operator = (const NonCopyable &) = delete;
#endif

protected:
  NonCopyable() {}
};

// Utility templates and functions.

/**
 * \brief Type branching template.
 * Defines its type Result depending on the value of \a Cond.
 *
 * \tparam Condition to evaluate on compile time.
 * \tparam A Type of \a Result if \a Cond is \c true.
 * \tparam B Type of \a Result if \a Cond is \c false.
 */
template <bool Cond, typename A, typename B>
struct TypeBranch;

/// Type branching template specialization for the \c true case.
template <typename A, typename B>
struct TypeBranch<true, A, B> {
  /// Same as the template parameter \a A.
  typedef A Result;
};

/// Type branching template specialization for the \c false case.
template <typename A, typename B>
struct TypeBranch<false, A, B> {
  /// Same as the template parameter \a B.
  typedef B Result;
};

/**
 * \brief Provides the type to use for const reference parameters in a smart way.
 *
 * The parameter will be passed by value instead as a const reference if
 * the type is smaller and the properties of the type allows it.
 *
 * \tparam T Type of the argument without any qualifications.
 */
template <typename T>
struct RParam {
  typedef typename TypeBranch<
      IsPOD<T>::value &&
      HasTrivialDefaultConstructor<T>::value &&
      HasTrivialCopyConstructor<T>::value &&
      HasTrivialDestructor<T>::value &&
      sizeof(T) <= sizeof(const T&), T, const T&>::Result Type;
};

/**
 * \brief Unsigned integer branching template.
 * Defines its value Result depending on the evaluation of \a Cond.
 *
 * \tparam Condition to evaluate on compile time.
 * \tparam A Value of \a Result if \a Cond is \c true.
 * \tparam B Value of \a Result if \a Cond is \c false.
 */
template <bool Cond, unsigned int A, unsigned int B>
struct UIntBranch;

/// Value branching template specialization for the \c true case.
template <unsigned int A, unsigned int B>
struct UIntBranch<true, A, B> {
  static const unsigned int value = A;
};

/// Value branching template specialization for the \c false case.
template <unsigned int A, unsigned int B>
struct UIntBranch<false, A, B> {
  static const unsigned int value = B;
};

/// Auxiliary template to find the minimum of 2 unsigned integers.
template <unsigned int a, unsigned int b>
struct Min : UIntBranch<a <= b, a, b> {};

/// Sets in \a value if the given value is a power of two.
template <unsigned int N>
struct IsPowerOfTwo {
  enum { value = (N && !(N & (N - 1))) ? true : false };
};

/// Returns if a value is a power of two.
bool is_power_of_two(unsigned int n) {
  return (n && !(n & (n - 1))) ? true : false;
}

/// Sets in \a value the next multiple of \a M >= \a N. Will rise a compile error if \a M is not a power of two.
template <unsigned int M, unsigned int N>
struct NextMultipleOfPOT {
  KCHE_TREE_COMPILE_ASSERT(IsPowerOfTwo<M>::value, "First parameter must be a power of two.");
  enum { value = (N + (M - 1)) & ~(M - 1) };
};

/// Returns the next multiple of \a m >= \a n. Will raise an assertion failure if \a m is not a power of two.
unsigned int next_multiple_of_pot(unsigned int m, unsigned int n) {
  KCHE_TREE_DCHECK(is_power_of_two(m));
  return (n + (m - 1)) & ~(m - 1);
}

} // namespace kche_tree

#endif
