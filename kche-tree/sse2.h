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
 * \file sse2.h
 * \brief Utility templates and types for SSE2-specific operations.
 * \author Leandro Graciá Gil
*/

#ifndef _KCHE_TREE_SSE2_H_
#define _KCHE_TREE_SSE2_H_

// Only if SSE2 is enabled an supported.
#if KCHE_TREE_ENABLE_SSE && KCHE_TREE_SSE2_SUPPORTED

// Functions for the SSE2 instruction set.
#include <emmintrin.h>
#include "sse.h"

namespace kche_tree {

/// SSE information for the double type.
template <>
struct SSETraits<double> {
  /// Define __m128d as the double precision floating point SSE register.
  typedef __m128d RegisterType;

  /// SSE __m128d registers contain 2 floating point values.
  static const unsigned int NumElements = 2;

  /// Return a register initialized to zero.
  static inline RegisterType zero() {
    return _mm_setzero_pd();
  }

  /// Return a register with one element initialized to the provided value.
  static inline RegisterType value(double value) {
    return _mm_set1_pd(value);
  }

  static inline RegisterType add(const RegisterType &a, const RegisterType &b) {
    return _mm_add_pd(a, b);
  }

  static inline RegisterType sub(const RegisterType &a, const RegisterType &b) {
    return _mm_sub_pd(a, b);
  }

  static inline RegisterType mult(const RegisterType &a, const RegisterType &b) {
    return _mm_mul_pd(a, b);
  }
};

} // namespace kche_tree

#endif // KCHE_TREE_ENABLE_SSE && KCHE_TREE_SSE2_SUPPORTED

#endif
