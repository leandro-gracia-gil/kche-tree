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
 * \file deleter.h
 * \brief Deletion functors for use in smart pointers and arrays.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_DELETER_H_
#define _KCHE_TREE_DELETER_H_

namespace kche_tree {

/// Default functor for deleting pointers.
template <typename T>
struct PointerDeleter {
  /// Deletes the provided pointer.
  void operator () (T *p) const { delete p; }
};

/// Default functor for deleting arrays.
template <typename T>
struct ArrayDeleter {
  /// Deletes the provided array.
  void operator () (T *p) const { delete []p; }
};

/// Functor for deleting shifted arrays. Useful for 1-indexed arrays like heaps.
template <typename T, unsigned int Shift>
struct ShiftedArrayDeleter {
  /// Deletes the provided shifted array.
  void operator () (T *p) const {
    if (p)
      delete [](p + Shift);
  }
};

/// Default functor for deleting aligned arrays constructed with the AlignedArray template.
template <typename T>
struct AlignedDeleter {
  // Implementation in aligned_array.h.
  void operator () (T *p) const;
};

} // namespace kche_tree

#endif
