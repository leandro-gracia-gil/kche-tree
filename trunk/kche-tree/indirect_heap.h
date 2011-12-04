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
 * \file indirect_heap.h
 * \brief Template for index-based indirect heaps.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_INDIRECT_HEAP_H_
#define _KCHE_TREE_INDIRECT_HEAP_H_

// Include the std::less predicate (used by default).
#include <functional>

#include "scoped_ptr.h"

namespace kche_tree {

/**
 * \brief Define a reference-based heap over an array of existing data that
 *        can be accessed and modified while mantaining the heap properties.
 *
 * Element indices can be pushed up to a maximum size.
 *
 * \tparam T Type of the data stored in the k-vector.
 * \tparam Compare Type of the comparison object. Defaults to std::less<T>.
 * \tparam Index Type used for encoding data vector indices. Defaults to unsigned int, but can be reduced to short or char for small K values.
*/
template <typename T, typename Compare = std::less<T>, typename Index = unsigned int>
class IndirectHeap {
public:

  // Constructors and destructors.
  IndirectHeap(T *data, unsigned int size, unsigned int max_size = 0, const Compare &c = Compare());
  IndirectHeap(const IndirectHeap &heap);

  // Assignment operator (only the heap structure is copied, not the data pointer).
  IndirectHeap & operator = (const IndirectHeap &heap);

  // Comparison operator (only heap structure is compared, not data pointers or data itself).
  bool operator == (const IndirectHeap &heap) const;

  // Data pointer adjustment.
  void set_data(T *data); ///< Update the pointer to data. Same size and structure is assumed. May require to rebuild the heap.

  // Usual heap operations.
  bool push(Index index); ///< Push an element into the heap. Cost: O(log n).
  T &pop(); ///< Pop the top element from the heap. Cost: O(log n).

  // Extended heap operations.
  bool in_heap(Index index) const; ///< Check if an object is the heap. Cost: O(1).
  bool remove(Index index); ///< Remove an object from the heap. Cost: O(log n).
  bool update(Index index); ///< Update the heap with the new value of an object. Cost: O(log n).
  void update_all(); ///< Build the heap stucture again. Cost: O(n).
  bool swap(Index index1, Index index2); ///< Update the heap to reflect the swap of two elements in the data array. Cost: O(1).

  // Subscript operators.
  const T & operator [](Index index) const;
  T & operator [](Index index);

  // Heap properties.
  unsigned int max_size() const;
  unsigned int data_size() const;
  unsigned int count() const;
  bool empty() const;
  Index top_index() const;
  T &top() const;

private:
  static const Index kRoot = 1; ///< Root index of the heap.

  mutable T *data_; ///< Array of stored elements. Shifted to be 1-indexed.
  ShiftedScopedArray<Index, kRoot> heap_; ///< Heap structure built as indices to data.
  ShiftedScopedArray<Index, kRoot> inverse_; ///< Inverse heap position indices.

  unsigned int size_; ///< Heap maximum size.
  unsigned int used_; ///< Number of elements used in the heap.
  unsigned int last_; ///< Position in the data array where last element is (1-indexed).

  Compare compare_; ///< Comparison object.

  inline Index parent_index(Index index) { return index >> 1; } ///< Get the index of the parent of a node.
  inline Index left_index (Index index) { return index << 1; } ///< Get the index of the left child of a node.
  inline Index right_index(Index index) { return (index << 1) + 1; } ///< Get the index of the right child of a node.

  inline void swap_elements(Index i1, Index i2); ///< Swap indices and references from a pair of elements.
  inline void heapify_element(Index index); ///< Adjust the heap structure after inserting a new element in \a index.
  bool heapify_upwards(Index index); ///< Adjust the heap structure upwards after inserting a new element in \a index.
  void heapify_downwards(Index index); ///< Adjust the heap structure downwards after inserting a new element in \a index.
};

} // namespace kche_tree

// Template implementation.
#include "indirect_heap.tpp"

#endif
