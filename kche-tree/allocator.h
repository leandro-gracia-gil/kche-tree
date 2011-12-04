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
 * \file allocator.h
 * \brief Memory allocator with optional alignment used by the kche-tree library.
 * \author Leandro Graciá Gil
 */

#ifndef KCHE_TREE_ALLOCATOR_H_
#define KCHE_TREE_ALLOCATOR_H_

// Includes for the SSE instruction set and aligned memory allocation.
#if !defined(__APPLE__)
#include <malloc.h>
#endif

namespace kche_tree {

/// Standard allocator. Performs no special operations.
template <bool align = Settings::enable_sse>
struct Allocator {
  static void *alloc(size_t nbytes) {
    void *p = malloc(nbytes);
    if (!p)
      throw std::bad_alloc();
    return p;
  }

  static void dealloc(void *p) {
    free(p);
  }
};

// Note: Mac OS X has already 16 byte memory alignment, so this specialization is not required.
#if !defined(__APPLE__)

/// Specialization to allocate memory aligned to 16 bytes. Required to use SSE instructions.
template <>
struct Allocator<true> {
  static void *alloc(size_t nbytes) {
    void *p = memalign(16, nbytes);
    if (!p)
      throw std::bad_alloc();
    return p;
  }

  static void dealloc(void *p) {
    free(p);
  }
};

#endif

}

#endif
