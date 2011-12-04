/***************************************************************************
 *   Copyright (C) 2010 by Leandro Graciá Gil                              *
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
 * \file k-heap.h
 * \brief Template for k-heaps holding the best k elements (logarithmic).
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_K_HEAP_H_
#define _KCHE_TREE_K_HEAP_H_

// Include the std::less predicate (used by default).
#include <functional>

// Include indirect heaps, traits information and optimized params.
#include "indirect_heap.h"
#include "scoped_ptr.h"
#include "traits.h"
#include "utils.h"

namespace kche_tree {

/**
 * \brief Define a reference-based heap that holds the K best elements on it allowing to push them in logarithmic time.
 *
 * \note Even if asyntotically optimal it could be slower than \link KVector k-vectors\endlink for small values of K due to extra operations and cache misses.
 *
 * \tparam T Type of the data stored in the k-heap.
 * \tparam Compare Type of the comparison object. The class must inherit from \c std::binary_function an define its \c bool (\c const \c &T, \c const \c &T) \c const operator.
*/
template <typename T, typename Compare = std::less<T> >
class KHeap {
public:
  /// Use optimized const reference types.
  typedef typename RParam<T>::Type ConstRef_T;

  // Constructors and destructors.
  KHeap(unsigned int K, const Compare& compare = Compare()); ///< K-heap constructor.
  KHeap(const KHeap &heap); ///< Copy constructor.

  // Assignment operator.
  KHeap & operator = (const KHeap &heap); ///< Assignment operator.

  // Comparison operator.
  bool operator == (const KHeap &heap) const; ///< Comparison operator.

  // Heap operations.
  bool push(ConstRef_T elem); ///< Push an element into the K-heap. Worst element will be replaced when heap is full. Cost: O(log K).
  void pop_best(); ///< Pop the best element from the heap. Can be used for direct sorting. Cost: O(log K).
  void pop_worst(); ///< Pop the worst element from the heap. Can be used for reverse sorting. Cost: O(log K).

  // Heap properties.
  bool full() const; ///< Check if the heap is full (it has already K elements). Cost: O(1).
  bool empty() const; ///< Check if the heap is empty. Cost: O(1).
  unsigned int size() const; ///< Return the number of elements in the heap. Cost: O(1).

  ConstRef_T best() const; ///< Retrieve the best element from the heap, or the first data object if empty. Cost: O(1).
  ConstRef_T worst() const; ///< Retrieve the worst element from the heap, or the first data object if empty. Cost: O(1).

  unsigned int K() const; ///< Get the maximum number of elements stored in the heap (also the maximum heap size). Cost: O(1).
  unsigned int count() const; ///< Get the number of elements currently in the heap. Cost: O(1).


  // STL-based names for heap operations and properties.
  bool push_front(ConstRef_T elem) { return push(elem); } ///< Same as \link KHeap::push push\endlink.
  bool push_back (ConstRef_T elem) { return push(elem); } ///< Same as \link KHeap::push push\endlink.

  void pop_front() { pop_worst(); } ///< Same as \link KHeap::pop_worst\endlink.
  void pop_back() { pop_best(); } ///< Same as \link KHeap::pop_best\endlink.

  ConstRef_T front() const { return worst();  } ///< Same as \link KHeap::worst\endlink.
  ConstRef_T back() const { return best(); } ///< Same as \link KHeap::best\endlink.

private:
  unsigned int K_; ///< Maximum number of best elements stored (also maximum heap size).
  ScopedArray<T> data_; ///< Array of stored elements (0-indexed).

  Compare compare_; ///< Comparison object used by internal heaps.
  IndirectHeap<T, Compare, unsigned int> best_heap_; ///< Heap storing best data indices.
  IndirectHeap<T, std::binary_negate<Compare>, unsigned int> worst_heap_; ///< Heap storing worst data indices.
};

} // namespace kche_tree

// Template implementation.
#include "k-heap.tpp"

#endif
