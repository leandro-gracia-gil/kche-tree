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
 * \file k-heap.tpp
 * \brief Template implementations for k-heaps holding the best k elements.
 * \author Leandro Graciá Gil
 */

// Include required C Standard Library STL files.
#include <algorithm>
#include <cstring>

namespace kche_tree {

/**
 * Build a k-heap of the given \a K size.
 *
 * \param K Maximum number of best elements to store in the heap.
 * \param compare Comparison object used to define an order.
*/
template <typename T, typename C>
KHeap<T, C>::KHeap(unsigned int K, const C &compare)
  : K_(K),
    data_(new T[K]),
    compare_(compare),
    best_heap_(data_.get(), 0, K, compare),
    worst_heap_(data_.get(), 0, K, std::binary_negate<C>(compare)) {}

/**
 * \brief Copy constructor.
 *
 * Initialize a KHeap copying another one.
 *
 * \param heap Element being copied.
 */
template <typename T, typename C>
KHeap<T, C>::KHeap(const KHeap &heap)
  : K_(heap.K_),
    data_(heap.K_ ? new T[heap.K_] : NULL),
    compare_(heap.compare_),
    best_heap_(heap.best_heap_),
    worst_heap_(heap.worst_heap_) {

  // Copy the data.
  if (data_)
    Traits<T>::copy_array(data_.get(), heap.data_.get(), K_);
}

/**
 * \brief Assignment operator.
 *
 * Copy the contents from another heap.
 *
 * \param heap Element being copied.
 */
template <typename T, typename C>
KHeap<T, C> &KHeap<T, C>::operator = (const KHeap &heap) {

  // Check self assignment.
  if (this == &heap)
    return *this;

  // Reallocate data if required.
  if (K_ != heap.K_)
    data_.reset(heap.K_ ? new T[heap.K_] : NULL);

  // Set the size.
  K_ = heap.K;

  // Set the comparison object.
  compare_ = heap.compare_;

  // Copy data.
  if (data_)
    Traits<T>::copy_array(data_.get(), heap.data_.get(), K_);

  // Copy heap structure.
  best_heap_ = heap.best_heap;
  worst_heap_ = heap.worst_heap;

  // Make them refer to our data.
  best_heap_.setData(data_.get());
  worst_heap_.setData(data_.get());

  // Return a reference to itself.
  return *this;
}

/**
 * \brief Comparison operator.
 *
 * Check if two heaps are identical in data an structure.
 *
 * \return \c true if equal, \c false otherwise.
 */
template <typename T, typename C>
bool KHeap<T, C>::operator == (const KHeap &heap) const {

  // Compare K value.
  if (K_ != heap.K_)
    return false;

  // Compare data.
  for (unsigned int i=0; i<K_; ++i)
    if (!(data_[i] == heap.data_[i]))
      return false;

  // Compare internal heaps.
  if (!(best_heap_ == heap.best_heap_))
    return false;
  if (!(worst_heap_ == heap.worst_heap_))
    return false;

  return true;
}

/**
 * Insert an element in the k-heap.
 *
 * \param elem Element to insert.
 * \return \c true if \a elem is kept in the heap, \c false if discarded.
*/
template <typename T, typename C>
bool KHeap<T, C>::push(ConstRef_T elem) {

  // Check if heaps are not yet full.
  if (best_heap_.count() < K_) {

    // Insert element into data vector.
    unsigned int next = best_heap_.count();
    data_[next] = elem;

    // Insert element in both heaps.
    best_heap_.push(next);
    worst_heap_.push(next);

    return true;
  }
  // Heaps are full: should try to replace worst element.
  else {
    // Check if new element is better than the actual worst one.
    if (compare_(elem, worst())) {

      // Replace worst element in the array.
      unsigned int worst_index = worst_heap_.top_index();
      data_[worst_index] = elem;

      // Update heaps.
      best_heap_.update(worst_index);
      worst_heap_.update(worst_index);

      return true;
    }
    // Discard new element.
    return false;
  }
}

/**
 * Extract the best element from the heap.
 * No value is returned to avoid unnecessary object copies.
*/
template <typename T, typename C>
void KHeap<T, C>::pop_best() {

  // Check size.
  if (best_heap_.empty())
    return;

  // Get the index of the best and the last elements.
  unsigned int best_index = best_heap_.top_index();
  unsigned int last_index = best_heap_.count() - 1;

  // Swap extracted element to the end in the data array and update heaps.
  if (best_index != last_index) {
    std::swap(data_[best_index], data_[last_index]);
    best_heap_.swap(best_index, last_index);
    worst_heap_.swap(best_index, last_index);
    std::swap(best_index, last_index);
  }

  // Remove the element from the worst elements heap.
  worst_heap_.remove(best_index);

  // Extract the topmost object from the best elements heap.
  best_heap_.pop();
}

/**
 * Extract the worst element from the heap.
 * No value is returned to avoid unnecessary object copies.
*/
template <typename T, typename C>
void KHeap<T, C>::pop_worst() {

  // Check size.
  if (worst_heap_.empty())
    return;

  // Get the index of the worst element.
  unsigned int worst_index = worst_heap_.top_index();
  unsigned int last_index = worst_heap_.count() - 1;

  // Swap extracted element to the end in the data array and update heaps.
  if (worst_index != last_index) {
    std::swap(data_[worst_index], data_[last_index]);
    best_heap_.swap(worst_index, last_index);
    worst_heap_.swap(worst_index, last_index);
    std::swap(worst_index, last_index);
  }

  // Remove the element from the best elements heap.
  best_heap_.remove(worst_index);

  // Extract the topmost object from the worst elements heap.
  worst_heap_.pop();
}

/**
 * Check if the heap is full.
 *
 * \return \c true if full, \c false if not.
*/
template <typename T, typename C>
bool KHeap<T, C>::full() const {
  return best_heap_.count() == K_;
}

/**
 * Check if the heap is empty.
 *
 * \return \c true if empty, \c false if not.
*/
template <typename T, typename C>
bool KHeap<T, C>::empty() const {
  return best_heap_.empty();
}

/**
 * Get the number of elements in the heap.
 *
 * \return Number of elements in the heap.
*/
template <typename T, typename C>
unsigned int KHeap<T, C>::size() const {
  return best_heap_.count();
}

/**
 * Get the current best element in the heap.
 *
 * \return Best element in the heap, or last 'first' element if heap is already \link KHeap::empty empty\endlink.
*/
template <typename T, typename C>
typename KHeap<T, C>::ConstRef_T KHeap<T, C>::best() const {
  if (empty())
    return data_[0];
  return best_heap_.top();
}

/**
 * Get the current worst element in the heap.
 *
 * \return Worst element in the heap, or last 'first' element if heap is already \link KHeap::empty empty\endlink.
*/
template <typename T, typename C>
typename KHeap<T, C>::ConstRef_T KHeap<T, C>::worst() const {
  if (empty())
    return data_[0];
  return worst_heap_.top();
}

/**
 * \brief Get the maximum number of elements stored in the heap (also the maximum heap size). Cost: O(1).
 *
 * \return The K value used to build the heap.
*/
template <typename T, typename C>
unsigned int KHeap<T, C>::K() const {
  return K_;
}

/**
 * \brief Get the number of elements currently in the heap. Cost: O(1).
 *
 * \return Current number of elements in the heap.
*/
template <typename T, typename C>
unsigned int KHeap<T, C>::count() const {
  return best_heap_.count();
}

} // namespace kche_tree
