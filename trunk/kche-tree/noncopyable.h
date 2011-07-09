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
 * \file noncopyable.h
 * \brief Base class for non-copyable objects and templates.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_NONCOPYABLE_H_
#define _KCHE_TREE_NONCOPYABLE_H_

namespace kche_tree {

/**
 * \brief Base class for non-copyable objects.
 *
 * Leaves undefined or deletes (if C++0x is enabled) the copy constructor and the asignment operator.
 */
class NonCopyable {
#ifdef KCHE_TREE_DISABLE_CPP0X
  NonCopyable(const NonCopyable &);
  NonCopyable &operator = (const NonCopyable &);
#else
  NonCopyable(const NonCopyable &) = delete;
  NonCopyable &operator = (const NonCopyable &) = delete;
#endif

protected:
  NonCopyable() {}
};

} // namespace kche_tree

#endif
