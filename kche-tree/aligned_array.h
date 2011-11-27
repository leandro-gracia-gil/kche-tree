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
 * \file aligned_array.h
 * \brief Memory-aligned array encapsulation templates.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_ALIGNED_ARRAY_H_
#define _KCHE_TREE_ALIGNED_ARRAY_H_

#include "allocator.h"
#include "deleter.h"

namespace kche_tree {

/**
 * \brief Encapsulates the allocation and management of an aligned memory array.
 *
 * Provides memory alignment requirements to arrays of objects of type T when the new []
 * operator for this type cannot be overloaded.
 *
 * This class is designed to be used with the ScopedAlignedArray and SharedAlignedArray classes
 * as a way to provide them with a properly aligned array. Basically, this class is a way
 * to enforce the correct allocation and deallocation usage by its syntactic rules.
 * Otherwise any mix or improper deletion of aligned arrays would lead to segmentation faults.
 *
 * \note This version of the class does not enforce any memory alignment and
 * just encapsulates the usual new [] and delete [] operators.
 *
 * \warning This class takes no ownership of the allocated array. It only allocates
 * the array and then it merely acts as a pointer encapsulation. No memory is replicated
 * or deleted when the object is either copied or destroyed.
 */
template <typename T, bool align = Settings::enable_sse>
class AlignedArray {
public:
  AlignedArray() : ptr_(NULL) {}
  explicit AlignedArray(unsigned int size) : ptr_(new T[size]) {}
  operator T *() const { return ptr_; }

  friend struct AlignedDeleter<T>;

private:
  static void release(T *p) { delete [](p); }
  T *ptr_;
};

/// Deletes an array previously created with the AlignedArray template.
template <typename T>
void AlignedDeleter<T>::operator () (T *p) const {
  AlignedArray<T>::release(p);
}

// -- Alignment specialization -- //

// Note: Mac OS X has already 16 byte memory alignment, so this specialization is not required.
#if !defined(__APPLE__)

/**
 * \brief 16 byte-aligned specialization of the AlignedArray class.
 *
 * Enables the use of arrays aligned to 16 bytes. This class makes internal use
 * of placement new and explicit invocation of destructors. Should only be used
 * with the ScopedAlignedArray and SharedAlignedArray objects. See the original
 * definition of the AlignedArray template for more details.
 *
 * \note It is safe to throw exceptions from the constructor of the type T during allocation.
 * Any objects will be destroyed and their memory will be properly released.
 */
template <typename T>
class AlignedArray<T, true> {
public:
  AlignedArray() : ptr_(NULL) {}
  explicit AlignedArray(unsigned int size);
  operator T *() const { return ptr_; }

  friend struct AlignedDeleter<T>;

private:
  /// Contains auxiliary extra data stored together with the array.
  struct ExtraData {
    unsigned int size;
  };

  static const unsigned int extras_offset = NextMultipleOfPOT<16, sizeof(ExtraData)>::value;

  static void release(T *p);
  static void destroy(void *base, T *ptr, unsigned int size);

  T *ptr_;
};

/**
 * \brief Allocate a memory-aligned array storing extra information and handling possible constructor exceptions.
 *
 * \param size Size of the array to be allocated.
 */
template <typename T>
AlignedArray<T, true>::AlignedArray(unsigned int size) {

  // Allocate the required memory plus an extra offset to store additional information about the array.
  void *base = Allocator<>::alloc(extras_offset + size * sizeof(T));
  ptr_ = reinterpret_cast<T *>(reinterpret_cast<unsigned char *>(base) + extras_offset);

  // Store the size of the array. This is needed to know how many destructors need to be called.
  ExtraData *extras = reinterpret_cast<ExtraData *>(base);
  extras->size = size;

  // Allocate each object in the array individually, but look out for exceptions.
  // In case of exception any previously constructed objects will be destructed and deallocated.
  unsigned int i = 0;
  try {
    for (; i<size; ++i)
      new (&ptr_[i]) T;
  } catch (...) {
    destroy(base, ptr_, i);
    throw;
  }
}

/**
 * \brief Delete a previously allocated aligned array. Invokes any required destructors.
 *
 * \param p Array to be released.
 */
template <typename T>
void AlignedArray<T, true>::release(T *p) {

  // As in delete and free, ignore NULL pointers.
  if (!p)
    return;

  // Destroy and delete all objects in the array.
  void *base = reinterpret_cast<unsigned char *>(p) - extras_offset;
  ExtraData *extras = reinterpret_cast<ExtraData *>(base);
  destroy(base, p, extras->size);
}

/**
 * \brief Destroy and release the contents of a memory-aligned array.
 *
 * \param base Real base address of the array. Points to the extra array information.
 * \param ptr Public address of the array. Points to the array's contents.
 * \param size Number of elements in the array.
 */
template <typename T>
void AlignedArray<T, true>::destroy(void *base, T *ptr, unsigned int size) {

  // Explicitly invoke the destructor of all objects in the proper order.
  while (size)
    ptr[--size].~T();

  // Deallocate the array.
  Allocator<>::dealloc(base);
}

#endif

} // namespace kche_tree

#endif
