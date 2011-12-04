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
 * \file indirect_heap.tpp
 * \brief Template implementations for index-based indirect heaps.
 * \author Leandro Graciá Gil
 */

// Include required C Standard Library STL files.
#include <algorithm>
#include <cassert>
#include <cstring>

namespace kche_tree {

/**
 * Build a mutable reference heap over an existing array.
 *
 * \param data Array with data.
 * \param size Number of elements in \a data.
 * \param max_size Maximum heap size (should fit in \a data).
 * \param compare Comparison object used to define an order.
 */
template <typename T, typename C, typename Index>
IndirectHeap<T, C, Index>::IndirectHeap(T *data, unsigned int size, unsigned int max_size, const C &compare)
  : compare_(compare) {

  // Check for input data.
  if (data == NULL) {
    data_ = NULL;
    size_ = 0;
    used_ = 0;
    last_ = 0;
    return;
  }

  // Initialize size and used counters.
  size_ = std::max<unsigned int>(size, max_size);
  used_ = size;
  last_ = used_;

  // Allocate auxiliar arrays. Shifted to make them 1-indexed.
  Index *array = new Index[size_];
  KCHE_TREE_DCHECK(array);
  heap_.reset(array - 1);

  array = new Index[size_];
  KCHE_TREE_DCHECK(array);
  inverse_.reset(array - 1);

  // Set data array (not owned, shifted).
  data_ = data - 1;

  // Initialize indices.
  for (unsigned int i=1; i<=size_; ++i)
    inverse_[i] = heap_[i] = i;

  // Build heap.
  for (unsigned int i=(used_ >> 1); i>=kRoot; --i)
    heapify_downwards(i);
}

/// Copy constructor. Heap structure and data pointer are copied from the input object.
template <typename T, typename C, typename Index>
IndirectHeap<T, C, Index>::IndirectHeap(const IndirectHeap &heap)
  : data_(NULL),
    size_(0),
    used_(0),
    last_(0),
    compare_(heap.compare) {

  // Use assignment operator internally.
  *this = heap;

  // Use the data pointer of the input object.
  data_ = heap.data_;
}

/// Assignment operator. Only heap structure is copied, not the data pointer.
template <typename T, typename C, typename Index>
IndirectHeap<T, C, Index> &IndirectHeap<T, C, Index>::operator = (const IndirectHeap &heap) {

  // Check self assignment.
  if (this == &heap)
    return *this;

  // Reallocate any previous arrays if sizes are different.
  if (size_ != heap.size_) {

    // Set size.
    size_ = heap.size_;

    // Reallocate arrays.
    Index *array = new Index[size_];
    KCHE_TREE_DCHECK(array);
    heap_.reset(array - 1);

    array = new Index[size_];
    KCHE_TREE_DCHECK(array);
    inverse_.reset(array - 1);
  }

  // Set heap attributes.
  used_ = heap.used_;
  last_ = heap.last_;

  // Copy array contents.
  memcpy(heap_.get() + 1, heap.heap_.get() + 1, size_ * sizeof(Index));
  memcpy(inverse_.get() + 1, heap.inverse_.get() + 1, size_ * sizeof(Index));

  // Return a reference to itself.
  return *this;
}

/// Comparison operator.
template <typename T, typename C, typename Index>
bool IndirectHeap<T, C, Index>::operator == (const IndirectHeap &heap) const {

  // Compare sizes, number of used elements.
  if (size_ != heap.size_ || used_ != heap.used_)
    return false;

  // Compare heap and inverse index arrays.
  if (memcmp(heap_.get() + 1, heap.heap_.get() + 1, size_ * sizeof(Index)) != 0)
    return false;
  if (memcmp(inverse_.get() + 1, heap.inverse_.get() + 1, size_ * sizeof(Index)) != 0)
    return false;

  return true;
}

/**
 * Swap element indices both in the heap and the inverse permutation arrays.
 *
 * \param i1 Index of the first element to swap (1-indexed).
 * \param i2 Index of the second element to swap (1-indexed).
 */
template <typename T, typename C, typename Index>
void IndirectHeap<T, C, Index>::swap_elements(Index i1, Index i2) {

  // Swap inverse indices.
  std::swap<Index>(inverse_[heap_[i1]], inverse_[heap_[i2]]);

  // Swap heap indices.
  std::swap<Index>(heap_[i1], heap_[i2]);
}

/**
 * Adjust an element position in the heap.
 *
 * \param index Index in the heap of the element to move (1-indexed).
 */
template <typename T, typename C, typename Index>
void IndirectHeap<T, C, Index>::heapify_element(Index index) {

  // Try upwards first (less comparisons) and if not modified (comparison pruning), try downwards.
  if (!heapify_upwards(index))
    heapify_downwards(index);
}

/**
 * Move an element upwards the heap.
 *
 * \param index Index in the heap of the element to move (1-indexed).
 * \return \c true if heap was modified, \c false otherwise.
 */
template <typename T, typename C, typename Index>
bool IndirectHeap<T, C, Index>::heapify_upwards(Index index) {

  // Keep swapping until heap condition is true or root node is reached.
  bool modified = false;
  while (index > kRoot) {

    // Get parent index.
    Index parent = parent_index(index);

    // Break if heap condition already true.
    if (compare_(data_[heap_[parent]], data_[heap_[index]]))
      break;

    // Swap elements in heap.
    modified = true;
    swap_elements(index, parent);

    // Move current index to its parent.
    index = parent;
  }

  return modified;
}

/**
 * Move an element downwards the heap.
 *
 * \param index Index in the heap of the element to move (1-indexed).
 */
template <typename T, typename C, typename Index>
void IndirectHeap<T, C, Index>::heapify_downwards(Index index) {

  // Keep swapping until heap condition is true or a leaf node is reached.
  while (index <= used_) {

    // Look for maximum element in parent and child nodes.
    Index max_index = index;
    Index left = left_index(index);
    if (left <= used_) {
      max_index = compare_(data_[heap_[left]], data_[heap_[index]]) ? left : max_index;
      Index right = right_index(index);
      if (right <= used_)
        max_index = compare_(data_[heap_[right]], data_[heap_[max_index]]) ? right : max_index;
    }

    // Break if heap condition already true.
    if (max_index == index)
      break;

    // Swap elements.
    swap_elements(index, max_index);

    // Move to child node.
    index = max_index;
  }
}

/**
 * Update the pointer to real data. Same size and structure is assumed. Rebuilding the heap may be required after this.
 * \c NULL pointers are ignored.
 *
 * \param data New pointer to data.
 */
template <typename T, typename C, typename Index>
void IndirectHeap<T, C, Index>::set_data(T *data) {

  // Shift data to make it 1-indexed.
  if (data)
    data_ = data - 1;
}

/**
 * Insert a new element in the heap, if it fits. Since data is never modified by the heap,
 * only existing data can be pushed into it, referenced by its index.
 *
 * \warning Only one copy of the same object can be in the heap at the same time.
 *
 * \param index Data array index of the element to push into the heap (0-indexed).
 * \return \c true if successful, \c false if heap is full or the element is already on the heap.
 */
template <typename T, typename C, typename Index>
bool IndirectHeap<T, C, Index>::push(Index index) {

  // Make index 1-indexed.
  ++index;

  // Check if index is in the array boundaries.
  if (index > size_ || size_ < kRoot)
    return false;

  // Get the index of the requested element in the heap.
  Index heap_index = inverse_[index];

  // Check if item is already in the heap.
  if (heap_index <= used_)
    return false;

  // Increase number of elements in use.
  ++used_;

  // Move requested item into the new heap boundaries.
  if (used_ != heap_index)
    swap_elements(used_, heap_index);

  // Move to its position in the heap.
  heapify_upwards(used_);

  return true;
}

/**
 * Extract topmost element from the heap.
 *
 * \return Topmost element being extracted, or first element if heap is already \link IndirectHeap::empty empty\endlink.
 */
template <typename T, typename C, typename Index>
T &IndirectHeap<T, C, Index>::pop() {

  // Check size.
  if (empty())
    return data_[kRoot];

  // Get a reference to the current top.
  T &topmost = data_[heap_[kRoot]];

  // Swap top and last elements.
  if (kRoot != used_)
    swap_elements(kRoot, used_);

  // Decrease the number of elements in the heap.
  --used_;

  // Adjust heap downwards from the root.
  heapify_downwards(kRoot);

  // Return stored reference.
  return topmost;
}

/**
 * Check if an element of the original array is still in the heap.
 *
 * \param index Index of the element to check (0-indexed).
 * \return \c true if still in the heap, \c false otherwise.
 */
template <typename T, typename C, typename Index>
bool IndirectHeap<T, C, Index>::in_heap(Index index) const {

  // Shift the index to make it 1-indexed.
  ++index;

  // Check boundaries and heap data.
  return index >= kRoot && index <= size_ && inverse_[index] <= used_;
}

/**
 * Access an element in the original array by its index. In case of modifying it,
 * \link IndirectHeap::update update\endlink should be called afterwards.
 *
 * \param index Index of the element to get (0-indexed).
 * \return A reference to the requested element, or to the last one in case of out-of-bounds request.
 */
template <typename T, typename C, typename Index>
T &IndirectHeap<T, C, Index>::operator [] (Index index) {

  // Shift the index to make it 1-indexed.
  ++index;

  // Get the corresponding element.
  if (index > size_)
    index = size_;
  return data_[index];
}

/**
 * Access an element in the original array by its index.
 *
 * \param index Index of the element to get (0-indexed).
 * \return A non-mutable reference to the requested element, or to the last one in case of out-of-bounds request.
 */
template <typename T, typename C, typename Index>
const T &IndirectHeap<T, C, Index>::operator [] (Index index) const {

  // Shift the index to make it 1-indexed.
  ++index;

  // Get the corresponding element.
  if (index > size_)
    index = size_;
  return data_[index];
}

/**
 * Remove the specified element from the heap.
 *
 * \param index Index of the element in data array being removed from the heap (0-indexed).
 * \return \c true if successful, \c false if \a index is not in array.
 */
template <typename T, typename C, typename Index>
bool IndirectHeap<T, C, Index>::remove(Index index) {

  // Check if the given index is inside the array (and shift it to make it 1-indexed).
  if (!in_heap(index++))
    return false;

  // Swap last and requested elements.
  Index heap_index = inverse_[index];
  if (heap_index != used_)
    swap_elements(heap_index, used_);

  // Decrease number of used elements.
  --used_;

  // Try to heapify in both directions (last element could come from a completely different branch).
  heapify_element(heap_index);

  return true;
}

/**
 * Update the heap structure after a item has been modified. Cost: O(log n)
 *
 * \param index Index of the item in the array that was modified (0-indexed).
 * \return \c true if successful, \c false if \a index is not in array.
 */
template <typename T, typename C, typename Index>
bool IndirectHeap<T, C, Index>::update(Index index) {

  // Check if the given index is inside the array (and shift it to make it 1-indexed).
  if (!in_heap(index++))
    return false;

  // Try to heapify in both directions (only one will).
  heapify_element(inverse_[index]);

  return true;
}

/**
 * Rebuild the heap structure completely. Designed to be used when many elements have been modified.
 * Cost: O(n), better than O(n log n) from n individual updates.
 */
template <typename T, typename C, typename Index>
void IndirectHeap<T, C, Index>::update_all() {

  // Reset indices.
  for (unsigned int i=1; i<=size_; ++i)
    inverse_[i] = heap_[i] = i;

  // Rebuild heap.
  for (unsigned int i=(used_ >> 1); i>=kRoot; --i)
    heapify_downwards(i);
}

/**
 * Update the heap after two elements were swapped from the data array. Cost: O(1).
 *
 * \param index1 Index of the first element in the data array (0-indexed).
 * \param index2 Index of the second element in the data array (0-indexed).
 * \return \c true if successful, \c false if any of the indices is not valid or not in the heap.
 */
template <typename T, typename C, typename Index>
bool IndirectHeap<T, C, Index>::swap(Index index1, Index index2) {

  // Check if both indices are in the heap.
  if (!in_heap(index1++))
    return false;
  if (!in_heap(index2++))
    return false;

  // Swap elements (works inverse to swap_elements internal method).
  std::swap<Index>(heap_[inverse_[index1]], heap_[inverse_[index2]]);
  std::swap<Index>(inverse_[index1], inverse_[index2]);

  return true;
}

/**
 * Get the data array index of the element being currently in the top of the heap.
 *
 * \return 0-based index of the topmost element in the heap, or first one if the heap is \link IndirectHeap::empty empty\endlink.
 */
template <typename T, typename C, typename Index>
Index IndirectHeap<T, C, Index>::top_index() const {
  if (empty())
    return 0;
  return heap_[kRoot] - 1;
}

/**
 * Get the element in the top of the heap.
 *
 * \return Reference to the element in the top of the heap, or to the first one if the heap is \link IndirectHeap::empty empty\endlink.
 */
template <typename T, typename C, typename Index>
T &IndirectHeap<T, C, Index>::top() const {
  if (empty())
    return data_[kRoot];
  return data_[heap_[kRoot]];
}

/**
 * Check if the heap is empty.
 *
 * \return \c true if empty, \c false if not.
 */
template <typename T, typename C, typename Index>
bool IndirectHeap<T, C, Index>::empty() const {
  return used_ == 0;
}

/**
 * Get maximum heap size.
 *
 * \return Maximum heap size.
 */
template <typename T, typename C, typename Index>
unsigned int IndirectHeap<T, C, Index>::max_size() const {
  return size_;
}

/**
 * Get the current number of elements in the data array (may be increased up to \link IndirectHeap::max_size max_size\endlink when \link IndirectHeap::push pushing\endlink data).
 *
 * \return Current number of elements in the data array.
 */
template <typename T, typename C, typename Index>
unsigned int IndirectHeap<T, C, Index>::data_size() const {
  return last_;
}


/**
 * Get the number of elements in the heap.
 *
 * \return Current number of elements in the heap.
 */
template <typename T, typename C, typename Index>
unsigned int IndirectHeap<T, C, Index>::count() const {
  return used_;
}

} // namespace kche_tree
