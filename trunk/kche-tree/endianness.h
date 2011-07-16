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
 * \file endianness.h
 * \brief Determine the endianess of the current platform.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_ENDIANESS_H_
#define _KCHE_TREE_ENDIANESS_H_

#ifdef KCHE_TREE_DISABLE_CPP0X
#include <tr1/cstdint>
#include <tr1/type_traits>
#else
#include <cstdint>
#include <type_traits>
#endif

// Include swap.
#include <algorithm>

// Include compile-time assertions.
#include "compile_assert.h"

namespace kche_tree {

/// Provide information about the endianness of the current platform.
struct Endianness {

  /// Enumeration type for the endianness.
  enum Type {
    BigEndian = 0, ///< Big endian.
    LittleEndian   ///< Little endian.
  };

  /// Get the endianness of the local host.
  static Type endianness() { static Type endianness = is_little_endian() ? LittleEndian : BigEndian; return endianness; }

  /// Check if the local host uses little endian.
  static bool is_little_endian() { static uint32_t x = 1; return (reinterpret_cast<const uint8_t*>(&x))[0]; }

  /// Check if the local host uses big endian.
  static bool is_big_endian() { return !is_little_endian(); }
};

/**
 * \brief Provide a generic endianness swap method for fundamental types of any size using template metaprogramming techniques.
 *
 * \tparam T Fundamental type to be swapped.
 * \tparam Iter Number of swapping iterations left.
 * \tparam Low Index of the low byte to swap.
 * \tparam High Index of the high byte to swap.
 */
template <typename T, unsigned int Iter = sizeof(T) / 2, uint32_t Low = 0, uint32_t High = sizeof(T) - 1>
struct EndiannessSwapper {
#ifdef KCHE_TREE_DISABLE_CPP0X
  KCHE_TREE_COMPILE_ASSERT(std::tr1::is_fundamental<T>::value, "EndiannessSwapper can only be applied to fundamental types");
#else
  KCHE_TREE_COMPILE_ASSERT(std::is_fundamental<T>::value, "EndiannessSwapper can only be applied to fundamental types");
#endif

  /**
   * \brief Run the bit swapping operation.
   *
   * \param value Address of the value to swap.
   */
  static void run(uint8_t *value) {
    std::swap(value[Low], value[High]);
    EndiannessSwapper<T, Iter - 1, Low + 1, High - 1>::run(value);
  }
};

/// Base case specialization for the byte swapping operation. Swap finished case.
template <typename T, unsigned int Low, unsigned int High>
struct EndiannessSwapper<T, 0, Low, High> {
  /// Nothing to do at this point.
  static void run(uint8_t *value) {}
};

} // namespace kche_tree

#endif
