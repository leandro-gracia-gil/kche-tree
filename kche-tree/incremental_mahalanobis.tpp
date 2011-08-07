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
 * \file incremental_mahalanobis.tpp
 * \brief Template implementations for incremental hyperrectangle intersection using the Mahalanobis metric.
 * \author Leandro Graciá Gil
 */

// Include symmetric matrices for the Mahalanobis inverse covariance.
#include "symmetric_matrix.h"

namespace kche_tree {

// Forward declarations.
template <typename T, const unsigned int D, bool isFundamental = IsFundamental<T>::value> struct MahalanobisIncrementalFunctor;

/**
 * \brief Functor for the Mahalanobis incremental distance update.
 * Especialization for fundamental types that makes use of operators that would require temporary copies in objects.
 */
template <typename T, const unsigned int D>
struct MahalanobisIncrementalFunctor<T, D, true> {

  /// Metric associated with this incremental calculation.
  typedef MahalanobisMetric<T, D> Metric;

  /// Use optimized const reference types.
  typedef typename RParam<T>::Type ConstRef_T;

  T& operator () (T &current_distance, unsigned int axis, ConstRef_T split_value,
      const KDSearchData<T, D, Metric> &search_data) const;
};

/**
 * \brief Functor for the Mahalanobis incremental distance update.
 * Especialization for non-fundamental types. Makes use only of the +=, -= and *= operators.
 */
template <typename T, const unsigned int D>
struct MahalanobisIncrementalFunctor<T, D, false> {

  /// Metric associated with this incremental calculation.
  typedef MahalanobisMetric<T, D> Metric;

  /// Use optimized const reference types.
  typedef typename RParam<T>::Type ConstRef_T;

  T& operator () (T &current_distance, unsigned int axis, ConstRef_T split_value,
      const KDSearchData<T, D, Metric> &search_data) const;
};

/**
 * \brief Incrementally update the distance from the implicit reference vector p to the current hyperrectangle using the Mahalanobis metric.
 *
 * This method follows the same foundations used in EuclideanIncrementalFunctor but calculating the squared distance as the result
 * of the multiplication of \a d \c x \a S \c x \a d' where \a S is the inverse covariance matrix associated with the metric.
 *
 * Because of the properties of the covariance matrices \a S is guaranteed to be symmetric. This method splits the actual incremental
 * calculation by separating the operations that involve the diagonal of \a S (when \a i == \a axis) and the operations that involve the rest
 * of the matrix requiring an additional loop. As a consequence, the operations can be easily simplified by an order of magnitude (relative
 * to the number of dimensions \a D) if the matrix \a S is diagonal, which is also a usual approximation when working with the Mahalanobis metric.
 *
 * \param current_distance Current distance to the hyperrectangle.
 * \param axis Number of the axis (dimension) being updated.
 * \param split_value Split value used to halve the hyperspace across the \a axis dimension.
 * \param search_data KD-tree search data information. Used to access the metric object in use.
 * \return The reference to the current distance to the hyperrectangle. Should have been updated.
 */
template <typename T, const unsigned int D>
T& MahalanobisIncrementalFunctor<T, D, true>::operator () (T &current_distance, unsigned int axis, ConstRef_T split_value, const KDSearchData<T, D, Metric> &search_data) const {

  const SymmetricMatrix<T> &S = search_data.metric.inverse_covariance();
  T inc_axis = search_data.axis[axis].nearest - split_value;
  T cur_axis = search_data.axis[axis].p - search_data.axis[axis].nearest;

  typedef typename Traits<T>::AccumulatorType AccumT;
  AccumT acc = S(axis, axis) * (inc_axis + cur_axis * 2.0);

  if (!search_data.metric.has_diagonal_covariance()) {
    for (unsigned int i=0; i<axis; ++i) {
      T cur_i = search_data.axis[i].p - search_data.axis[i].nearest;
      acc += cur_i * S(i, axis) * 2.0;
    }

    for (unsigned int i=axis+1; i<D; ++i) {
      T cur_i = search_data.axis[i].p - search_data.axis[i].nearest;
      acc += cur_i * S(axis, i) * 2.0;
    }
  }

  return current_distance += acc * inc_axis;
}

/**
 * \brief Incrementally update the distance from the implicit reference vector p to the current hyperrectangle using the Mahalanobis metric.
 * Specialization for non-fundamental types that avoids unnecessary copies of objects.
 *
 * See the documentation for the template parameter \c isFundamental = \c true specialization for details.
 */
template <typename T, const unsigned int D>
T& MahalanobisIncrementalFunctor<T, D, false>::operator () (T &current_distance, unsigned int axis, ConstRef_T split_value, const KDSearchData<T, D, Metric> &search_data) const {

  const SymmetricMatrix<T> &S = search_data.metric.inverse_covariance();
  T inc_axis = search_data.axis[axis].nearest;
  inc_axis -= split_value;
  T cur_axis = search_data.axis[axis].p;
  cur_axis -= search_data.axis[axis].nearest;

  typedef typename Traits<T>::AccumulatorType AccumT;
  AccumT acc = cur_axis;
  acc += cur_axis;
  acc += inc_axis;
  acc *= S(axis, axis);

  if (!search_data.metric.has_diagonal_covariance()) {
    for (unsigned int i=0; i<axis; ++i) {
      T temp = search_data.axis[i].p;
      temp -= search_data.axis[i].nearest;
      temp += temp;
      temp *= S(i, axis);
      acc += temp;
    }

    for (unsigned int i=axis+1; i<D; ++i) {
      T temp = search_data.axis[i].p;
      temp -= search_data.axis[i].nearest;
      temp += temp;
      temp *= S(axis, i);
      acc += temp;
    }
  }

  acc *= inc_axis;
  return current_distance += acc;
}

/// Update the current incremental distance using the Mahalanobis metric.
template <typename T, const unsigned int D>
MahalanobisIncrementalUpdater<T, D>::MahalanobisIncrementalUpdater(const KDNode<T, D> *node, const KDNode<T, D> *parent, KDSearchData<T, D, Metric> &search_data)
    : IncrementalBase<T, D, Metric>(search_data) {
  IncrementalBase<T, D, Metric>::update(node, parent, search_data, MahalanobisIncrementalFunctor<T, D>());
}

/// Undo any incremental updates performed to the hyperrectangle distance.
template <typename T, const unsigned int D>
MahalanobisIncrementalUpdater<T, D>::~MahalanobisIncrementalUpdater() {
  IncrementalBase<T, D, Metric>::restore();
}

} // namespace kche_tree
