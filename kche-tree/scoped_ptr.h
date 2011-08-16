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
 * \file scoped_ptr.h
 * \brief Define a basic scoped pointer template if C++0x is not available.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_SCOPED_PTR_H_
#define _KCHE_TREE_SCOPED_PTR_H_

#include "noncopyable.h"

namespace kche_tree {

#ifdef KCHE_TREE_DISABLE_CPP0X
/**
 * \brief Implement a basic version of scoped pointers.
 *
 * This doesn't intend to be a full implementation, but a basic version
 * to keep compatibility with the non-C++0x code.
 *
 * \tparam T Type of the smart pointer.
 * \tparam Del Functor providing deletion. Defaults to a standard pointer deleter.
 */
template <typename T, typename Del = PointerDeleter<T> >
class ScopedPtr : NonCopyable {
public:
  typedef T ElementType; ///< Type of the pointer being handled.

  explicit ScopedPtr(T *ptr = 0, const Del &deleter = Del()) : NonCopyable(), ptr_(ptr), deleter_(deleter) {}
  ~ScopedPtr() { deleter_(ptr_); }

  T *get() const { return ptr_; }
  T &operator *() const { assert(ptr_); return *ptr_; }
  T *operator ->() const { assert(ptr_); return ptr_; }

  typedef T *(ScopedPtr::*UnspecifiedBoolType);
  operator UnspecifiedBoolType() const { return ptr_ ? &ScopedPtr::ptr_ : 0; }
  bool operator !() const { return !ptr_; }

  void reset(T *ptr = 0) {
    if (ptr != ptr_) {
      deleter_(ptr_);
      ptr_ = ptr;
    }
  }

  void swap(ScopedPtr &p) { std::swap(ptr_, p.ptr_); }

protected:
  T *ptr_;
  Del deleter_;
};

/**
 * \brief Implement a basic version of scoped arrays.
 *
 * This doesn't intend to be a full implementation, but a basic version
 * to keep compatibility with the non-C++0x code.
 *
 * \tparam T Type of the smart pointer.
 */
template <typename T>
class ScopedArray : public ScopedPtr<T, ArrayDeleter<T> > {
public:
  explicit ScopedArray(T *ptr = 0) : ScopedPtr<T, ArrayDeleter<T> >(ptr, ArrayDeleter<T>()) {}

  const T &operator [](size_t index) const { return this->ptr_[index]; }
  T &operator [](size_t index) { return this->ptr_[index]; }
};

#else
/**
 * \brief Extended version of STL unique_ptr for compatibility with the non-C++0x code.
 *
 * \tparam T Type of the smart pointer.
 */
template <typename T>
class ScopedPtr : public std::unique_ptr<T> {
public:
  typedef T ElementType; ///< Type of the pointer being handled.
  explicit ScopedPtr(T *ptr = 0) : std::unique_ptr<T>(ptr) {}
};

/**
 * \brief Extended version of STL unique_ptr applied to arrays. Defined for compatibility with the non-C++0x code.
 *
 * STL unique_ptr has support for arrays via the partial specification of T[].
 * Likely to be replaced by a C++0x template alias when available.
 *
 * \tparam T Type of the smart pointer.
 */
template <typename T>
class ScopedArray : public std::unique_ptr<T[]> {
public:
  typedef T ElementType; ///< Type of the pointer being handled.
  explicit ScopedArray(T *ptr = 0) : std::unique_ptr<T[]>(ptr) {}
};

#endif

/// Swap the contents of two scoped pointers.
template <typename T>
void swap(ScopedPtr<T> &p1, ScopedPtr<T> &p2) { p1.swap(p2); }

/// Swap the contents of two scoped arrays.
template <typename T>
void swap(ScopedArray<T> &p1, ScopedArray<T> &p2) { p1.swap(p2); }

} // namespace kche_tree

#endif
