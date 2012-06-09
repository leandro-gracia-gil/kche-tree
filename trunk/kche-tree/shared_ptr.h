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
 * \file shared_ptr.h
 * \brief Define aliases for reference-counted shared pointers either from C++ TR1 or C++1x STL.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_SHARED_PTR_H_
#define _KCHE_TREE_SHARED_PTR_H_

#include "cpp1x.h"
#include "deleter.h"
#include "aligned_array.h"

namespace kche_tree {

/**
 * \brief Provide a basic interface to shared (reference-counted) pointers.
 *
 * \tparam T Type of the smart pointer.
 * \tparam Base Internal parameter used to derive either from C++ TR1 or from C++1x STL.
 */
template <typename T, typename Base = StdSharedPtr<T> >
class SharedPtr : public Base {
public:
  typedef PointerDeleter<T> DeleterType;
  typedef T ElementType; ///< Type of the pointer being handled.
  explicit SharedPtr(T *ptr = 0) : Base(ptr, DeleterType()) {}
};

/**
 * \brief Provide a basic interface to shared (reference-counted) arrays.
 *
 * \tparam T Type of the smart pointer.
 * \tparam Base Internal parameter used to derive either from C++ TR1 or from C++1x STL.
 */
template <typename T, typename Base = StdSharedPtr<T> >
class SharedArray : public Base {
public:
  typedef T ElementType; ///< Type of the pointer being handled.
  explicit SharedArray(T *ptr = 0) : Base(ptr, Deleter()) {}
  void reset(T *ptr = 0) { Base::reset(ptr, Deleter()); }

  const T & operator [](size_t index) const { return (this->get())[index]; }
  T & operator [](size_t index) { return (this->get())[index]; }

private:
  typedef ArrayDeleter<T> Deleter;
};

/**
 * \brief Provide a basic interface to memory-aligned shared (reference-counted) arrays.
 *
 * Handles and deletes memory-aligned arrays allocated using the AlignedArray template.
 *
 * \tparam T Type of the smart pointer.
 * \tparam Base Internal parameter used to derive either from C++ TR1 or from C++1x STL.
 */
template <typename T, typename Base = StdSharedPtr<T> >
class SharedAlignedArray : public Base {
public:
  typedef T ElementType; ///< Type of the pointer being handled.

  explicit SharedAlignedArray(AlignedArray<T> &ptr = AlignedArray<T>()) : Base(ptr.get(), Deleter()) {}
  void reset(AlignedArray<T> &ptr = AlignedArray<T>()) { Base::reset(ptr, Deleter()); }

  const T & operator [](size_t index) const { return (this->get())[index]; }
  T & operator [](size_t index) { return (this->get())[index]; }

private:
  typedef AlignedArray<T> Deleter;
};

/// Swap the contents of two shared pointers.
template <typename T>
void swap(SharedPtr<T> &p1, SharedPtr<T> &p2) { p1.swap(p2); }

/// Swap the contents of two shared arrays.
template <typename T>
void swap(SharedArray<T> &p1, SharedArray<T> &p2) { p1.swap(p2); }

/// Swap the contents of two shared aligned arrays.
template <typename T>
void swap(SharedAlignedArray<T> &p1, SharedAlignedArray<T> &p2) { p1.swap(p2); }

} // namespace kche_tree

#endif
