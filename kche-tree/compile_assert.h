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
 * \file compile_assert.h
 * \brief Provide compile-time assertions to check invalid template instantiations and conditions.
 * \author Leandro Graciá Gil
*/

#ifndef _KCHE_TREE_COMPILE_ASSERT_H_
#define _KCHE_TREE_COMPILE_ASSERT_H_

namespace kche_tree {

/// Auxiliary struct to provide compile-time assertions. You should see this name on an error if the assertion fails.
template <bool fail = false> struct COMPILE_ASSERT_FAILURE;

/// Auxiliary struct to provide compile-time assertions. You should see this name on an error if the assertion fails.
template <> struct COMPILE_ASSERT_FAILURE<true> { enum { x = 1 }; };

/// Auxiliary struct to provide compile-time assertions.
template <size_t> struct CompileAssertTest {};

/// Macro used to check assertions in compile time.
#define KCHE_TREE_COMPILE_ASSERT(x) \
  typedef ::kche_tree::CompileAssertTest< \
  sizeof(::kche_tree::COMPILE_ASSERT_FAILURE<(bool)(x)>)> _CompileAssertType

} // namespace kche_tree

#endif
