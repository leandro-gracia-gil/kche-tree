/***************************************************************************
 *   Copyright (C) 2012 by Leandro Graciá Gil                              *
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
 * \file cpp1x.h
 * \brief Defines wrappers around C++ TR1 or C++1x as available.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_CPP1X_H_
#define _KCHE_TREE_CPP1X_H_

#ifdef KCHE_TREE_DISABLE_CPP1X
#include <tr1/cstdint>
#include <tr1/memory>
#include <tr1/random>
#include <tr1/type_traits>
#else
#include <cstdint>
#include <memory>
#include <random>
#include <type_traits>
#endif

namespace kche_tree {

/// Wrapper around C++ TR1 / C++1x is_base_of.
template <typename T, typename U>
struct IsBaseOf :
#ifdef KCHE_TREE_DISABLE_CPP1X
  std::tr1::is_base_of<T, U>
#else
  std::is_base_of<T, U>
#endif
{};

/// Wrapper around C++ TR1 / C++1x is_arithmetic.
template <typename T>
struct IsArithmetic :
#ifdef KCHE_TREE_DISABLE_CPP1X
  std::tr1::is_arithmetic<T>
#else
  std::is_arithmetic<T>
#endif
{};

/// Wrapper around C++ TR1 / C++1x is_fundamental.
template <typename T>
struct IsFundamental :
#ifdef KCHE_TREE_DISABLE_CPP1X
  std::tr1::is_fundamental<T>
#else
  std::is_fundamental<T>
#endif
{};

/// Wrapper around C++ TR1 / C++1x is_integral.
template <typename T>
struct IsIntegral :
#ifdef KCHE_TREE_DISABLE_CPP1X
  std::tr1::is_integral<T>
#else
  std::is_integral<T>
#endif
{};

/// Wrapper around C++ TR1 / C++1x is_pod.
template <typename T>
struct IsPOD :
#ifdef KCHE_TREE_DISABLE_CPP1X
  std::tr1::is_pod<T>
#else
  std::is_pod<T>
#endif
{};

/// Wrapper around C++ TR1 / C++1x is_same.
template <typename T, typename U>
struct IsSame :
#ifdef KCHE_TREE_DISABLE_CPP1X
  std::tr1::is_same<T, U>
#else
  std::is_same<T, U>
#endif
{};

/// Wrapper around C++ TR1 / C++1x has_trivial_default_constructor.
template <typename T>
struct HasTrivialDefaultConstructor :
#ifdef KCHE_TREE_DISABLE_CPP1X
  std::tr1::has_trivial_constructor<T>
#else
  std::has_trivial_default_constructor<T>
#endif
{};

/// Wrapper around C++ TR1 / C++1x has_trivial_copy and has_trivial_copy_constructor.
template <typename T> struct HasTrivialCopyConstructor :
#ifdef KCHE_TREE_DISABLE_CPP1X
  std::tr1::has_trivial_copy<T>
#else
  std::has_trivial_copy_constructor<T>
#endif
{};

/// Wrapper around C++ TR1 / C++1x has_trivial_destructor.
template <typename T>
struct HasTrivialDestructor :
#ifdef KCHE_TREE_DISABLE_CPP1X
  std::tr1::has_trivial_destructor<T>
#else
  std::has_trivial_destructor<T>
#endif
{};

/// Wrapper around C++ TR1 / C++1x shared_ptr.
template <typename T> struct StdSharedPtr :
#ifdef KCHE_TREE_DISABLE_CPP1X
  std::tr1::shared_ptr<T>
#else
  std::shared_ptr<T>
#endif
{
  typedef
#ifdef KCHE_TREE_DISABLE_CPP1X
    std::tr1::shared_ptr<T>
#else
    std::shared_ptr<T>
#endif
    Base;

  // Wrapper constructors.
  template <typename U> explicit StdSharedPtr(U *p) : Base(p) {}
  template <typename U, typename Deleter> StdSharedPtr(U *p, Deleter d) : Base(p, d) {}
};

/// Wrapper around the default random engine to use.
typedef
#ifdef KCHE_TREE_DISABLE_CPP1X
  std::tr1::minstd_rand
#else
  std::default_random_engine
#endif
  DefaultRandomEngine;

/// Wrapper around C++ TR1 / C++1x uniform_int and uniform_int_distribution.
template <typename T> struct UniformInt :
#ifdef KCHE_TREE_DISABLE_CPP1X
  std::tr1::uniform_int<T>
#else
  std::uniform_int_distribution<T>
#endif
{
  typedef
#ifdef KCHE_TREE_DISABLE_CPP1X
    std::tr1::uniform_int<T>
#else
    std::uniform_int_distribution<T>
#endif
    Base;

  typedef T Element;

  UniformInt(T a, T b) : Base(a, b) {}

  template <typename RandomEngine>
  T operator ()(RandomEngine &engine) { return Base::operator ()(engine); }
};

/// Wrapper around C++ TR1 / C++1x uniform_real and uniform_real_distribution.
template <typename T> struct UniformReal :
#ifdef KCHE_TREE_DISABLE_CPP1X
  std::tr1::uniform_real<T>
#else
  std::uniform_real_distribution<T>
#endif
{
  typedef
#ifdef KCHE_TREE_DISABLE_CPP1X
    std::tr1::uniform_real<T>
#else
    std::uniform_real_distribution<T>
#endif
    Base;

  typedef T Element;

  UniformReal(T a, T b) : Base(a, b) {}

  template <typename RandomEngine>
  T operator ()(RandomEngine &engine) { return Base::operator ()(engine); }
};

/// Wrapper around C++ TR1 / C++1x random engine and distribution bindings.
template <typename RandomEngine, typename RandomDistribution> class RandomGenerator {
public:
  RandomGenerator(RandomEngine &engine, const RandomDistribution &distribution) :
#ifdef KCHE_TREE_DISABLE_CPP1X
      generator(engine, distribution)
#else
      generator(std::bind(distribution, std::ref(engine)))
#endif
  {}

  typedef typename RandomDistribution::Element Element;

  Element operator ()() { return generator(); }

private:
#ifdef KCHE_TREE_DISABLE_CPP1X
  std::tr1::variate_generator<RandomEngine &, RandomDistribution> generator;
#else
  std::function<Element()> generator;
#endif
};

// Wrapper around static assertions.
#ifdef KCHE_TREE_DISABLE_CPP1X

/// Auxiliary struct to provide compile-time assertions. You should see this name on an error if the assertion fails.
template <bool fail = false> struct COMPILE_ASSERT_FAILURE;

/// Auxiliary struct to provide compile-time assertions. You should see this name on an error if the assertion fails.
template <> struct COMPILE_ASSERT_FAILURE<true> { enum { x = 1 }; };

/// Auxiliary struct to provide compile-time assertions.
template <size_t> struct CompileAssertTest {};

#define KCHE_TREE_MACRO_CONCATENATE(x, y) x ## y
#define KCHE_TREE_MACRO_JOIN(x, y) KCHE_TREE_MACRO_CONCATENATE(x, y)

#define KCHE_TREE_COMPILE_ASSERT_INTERNAL(x, msg) \
  typedef ::kche_tree::CompileAssertTest< \
  sizeof(::kche_tree::COMPILE_ASSERT_FAILURE<(bool)(x)>)> _CompileAssertType_

// Raises a compile-time assertion if x doesn't evaluate to true.
// The message argument is discarded in the non-C++1x version.
#define KCHE_TREE_COMPILE_ASSERT(x, msg) \
  KCHE_TREE_MACRO_JOIN(KCHE_TREE_COMPILE_ASSERT_INTERNAL(x, msg), __COUNTER__)

#else
// Raises a compile-time assertion if x doesn't evaluate to true.
#define KCHE_TREE_COMPILE_ASSERT(x, msg) static_assert((x), msg)
#endif

} // namespace kche_tree

#endif
