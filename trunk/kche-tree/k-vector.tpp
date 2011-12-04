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
 * \file k-vector.tpp
 * \brief Template implementations for k-vectors holding the best k elements (linear, but simpler).
 * \author Leandro Graciá Gil
 */

namespace kche_tree {

/**
 * Build a k-vector of size K.
 *
 * \param K Size of the k-vector.
 * \param c Comparison object used for internal element sorting.
 */
template <typename T, typename Compare>
KVector<T, Compare>::KVector(unsigned int K, const Compare &c)
    : data_(new T[K]),
      K_(K),
      stored_(0),
      compare_(c) {}

/// Copy constructor for k-vector objects.
template <typename T, typename Compare>
KVector<T, Compare>::KVector(const KVector &vector)
    : data_(vector.K_ ? new T[vector.K_] : NULL),
      K_(vector.K_),
      stored_(vector.stored_),
      compare_(vector.compare_) {

  if (data_)
    Traits<T>::copy_array(data_.get(), vector.data_.get(), K_);
}

/// Assignment operator for k-vector objects.
template <typename T, typename Compare>
KVector<T, Compare> & KVector<T, Compare>::operator = (const KVector &vector) {

  // Check self assignment.
  if (this == &vector)
    return *this;

  if (K_ != vector.K_)
      data_.reset(vector.K_ ? new T[vector.K_] : NULL);

  K_ = vector.K_;
  stored_ = vector.stored_;
  compare_ = vector.compare_;

  // Copy data.
  if (data_)
    Traits<T>::copy_array(data_.get(), vector.data_.get(), K_);

  return *this;
}

/**
 * Check if no elements are being stored in the k-vector.
 *
 * \return \c true if the k-vector is empty, \c false otherwise.
 */
template <typename T, typename Compare>
bool KVector<T, Compare>::empty() const {
  return stored_ == 0;
}

/**
 * Check if K elements are being stored in the k-vector with K being the maximum size.
 *
 * \return \c true if the k-vector is full, \c false otherwise.
 */
template <typename T, typename Compare>
bool KVector<T, Compare>::full() const {
  return stored_ == K_;
}

/**
 * Get the current number of elements in the k-vector.
 *
 * \return Number of elements currently in the k-vector.
 */
template <typename T, typename Compare>
unsigned int KVector<T, Compare>::size() const {
  return stored_;
}

/**
 * Get the worst element currently stored in the k-vector.
 *
 * \return Reference to the worst element stored.
 */
template <typename T, typename Compare>
typename KVector<T, Compare>::ConstRef_T  KVector<T, Compare>::front() const {
  return data_[0];
}

/**
 * Get the best element currently stored in the k-vector.
 *
 * \return Reference to the best element stored.
 */
template <typename T, typename Compare>
typename KVector<T, Compare>::ConstRef_T  KVector<T, Compare>::back() const {
  return data_[stored_ - 1];
}

/**
 * Pop the best element stored in the k-vector.
 */
template <typename T, typename Compare>
void KVector<T, Compare>::pop_back() {
  if (stored_ > 0)
    --stored_;
}

/**
 * Push a new element into the k-vector.
 *
 * \param elem Element being pushed.
 */
template <typename T, typename Compare>
void KVector<T, Compare>::push_back(ConstRef_T elem) {
  if (stored_ < K_)
    push_not_full(elem);
  else
    push_full(elem);
}

/**
 * Push a new element into the k-vector with the precondition that it's still not full.
 * Ensuring the precondition before calling is responsability of the caller.
 * For a generic pushing method use \link KVector::push_back push_back\endlink.
 *
 * \pre The k-vector object must not be full.
 *
 * \param elem Element being pushed.
 */
template <typename T, typename Compare>
void KVector<T, Compare>::push_not_full(ConstRef_T elem) {

  // Look where the new candidate should be placed.
  unsigned int index;
  for (index=0; index < stored_ && compare_(elem, data_[index]); ++index);

  // Move later candidates and store the new one in its place.
  for (unsigned int i=stored_; i>index; --i)
    data_[i] = data_[i-1];
  data_[index] = elem;
  ++stored_;
}

/**
 * Push a new element into the k-vector with the precondition that it's already full.
 * Ensuring the precondition before calling is responsability of the caller.
 * For a generic pushing method use \link KVector::push_back push_back\endlink.
 *
 * \pre The k-vector object must be already full.
 *
 * \param elem Element being pushed.
 */
template <typename T, typename Compare>
void KVector<T, Compare>::push_full(ConstRef_T elem) {

  // Avoid further calculations if candidate is worst than the current worst one.
  if (!compare_(elem, data_[0]))
    return;

  // Look where the new candidate should be placed.
  unsigned int index;
  for (index=1; index < stored_ && compare_(elem, data_[index]); ++index);

  // Move previous candidates and store the new one in its place.
  --index;
  for (unsigned int i=0; i<index; ++i)
    data_[i] = data_[i+1];
  data_[index] = elem;
}

} // namespace kche_tree
