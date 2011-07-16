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
 * \file smart_ptr.h
 * \brief Define aliases for smart pointers either from C++ TR1 or C++0x STL.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_SMART_PTR_H_
#define _KCHE_TREE_SMART_PTR_H_

#ifdef KCHE_TREE_DISABLE_CPP0X
#include <tr1/memory>
#else
#include <memory>
#endif

namespace kche_tree {

/// Default functor for deleting pointers.
template <typename T>
struct PointerDeleter {
  void operator () (T *p) const { delete p; }
};

/// Default functor for deleting arrays.
template <typename T>
struct ArrayDeleter {
  void operator () (T *p) const { delete []p; }
};

} // namespace kche_tree

// Scoped and shared pointers.
#include "scoped_ptr.h"
#include "shared_ptr.h"

#endif
