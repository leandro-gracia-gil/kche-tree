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
 * \file incremental_euclidean.tpp
 * \brief Template implementations for incremental hyperrectangle intersection using the Euclidean metric.
 * \author Leandro Graciá Gil
 */

namespace kche_tree {

// Forward declarations.
template <typename T, const unsigned int D, bool isFundamental = IsFundamental<T>::value> struct EuclideanIncrementalFunctor;

/**
 * \brief Functor for the Euclidean incremental distance update.
 * Especialization for fundamental types that makes use of operators that would require temporary copies in objects.
 */
template <typename T, const unsigned int D>
struct EuclideanIncrementalFunctor<T, D, true> {

  /// Metric associated with this incremental calculation.
  typedef EuclideanMetric<T, D> Metric;

  /// Use optimized const reference types.
  typedef typename RParam<T>::Type ConstRef_T;

  T& operator () (T &current_distance, unsigned int axis, ConstRef_T split_value,
      const KDSearchData<T, D, Metric> &search_data) const;
};

/**
 * \brief Functor for the Euclidean incremental distance update.
 * Especialization for non-fundamental types. Makes use only of the +=, -= and *= operators.
 */
template <typename T, const unsigned int D>
struct EuclideanIncrementalFunctor<T, D, false> {

  /// Metric associated with this incremental calculation.
  typedef EuclideanMetric<T, D> Metric;

  /// Use optimized const reference types.
  typedef typename RParam<T>::Type ConstRef_T;

  T& operator () (T &current_distance, unsigned int axis, ConstRef_T split_value,
      const KDSearchData<T, D, Metric> &search_data) const;
};

/**
 * \brief Incrementally update the distance from the implicit reference vector p to the current hyperrectangle using the Euclidean metric.
 *
 * The folowing is an explanation of the mathematical foundations used here.
 *
 * Let \a i = axis be the dimension that has changed. Let \a p_i be the value of the i-th dimension of the reference vector \a p.
 * Let \a nearest_i be the value of the \a i-th dimension of the nearest point to \a p inside the current hyperrectangle.
 * Let \a split_i be the split value from the parent node that halves the hyperspace in the i-th dimension.
 *
 * The idea is that during the exploration the current hyperrectangle (defined by the node being explored) changes and consequently the nearest
 * distance to it might change. This method will be called only when this happens. Since the updates are always performed on a single axis,
 * this method calculates the appropriate increment to the total squared distance given the local axis increment.
 * This local increment can be defined as \a inc_i = \a nearest_i - \a split_i.
 *
 * In the case of the Euclidean metric, the squared distance is the sum of the squared differences of p and nearest along all the dimensions.
 * Calling these current per-axis differences \a d_i, then the current distance = \a d_0^2 + \a d_1^2 + ... + \a d_D-1^2.
 *
 * If the i-th dimension is increased by inc_i then the new distance = \a d_0^2 + ... + (\a d_i + \a inc_i)^2 + ... + \a d_D-1^2.
 * The difference between both distances is \a inc^2 + 2 * \a d_i * \a inc_i, which refactored equals to \a inc_i * (\a inc_i + 2 * \a d_i).
 *
 * Having \a inc_i = \a nearest_i - \a split_i and \a d_i = \a p_i - \a nearest_i, the contents are just a simplification of the previous formula.
 *
 * \param current_distance Current distance to the hyperrectangle.
 * \param axis Number of the axis (dimension) being updated.
 * \param split_value Split value used to halve the hyperspace across the \a axis dimension.
 * \param search_data KD-tree search data information. Currently not required by the Euclidean metric.
 * \return The reference to the current distance to the hyperrectangle. Should have been updated.
 */
template <typename T, const unsigned int D>
T& EuclideanIncrementalFunctor<T, D, true>::operator () (T &current_distance, unsigned int axis, ConstRef_T split_value, const KDSearchData<T, D, Metric> &search_data) const {
  const typename IncrementalBase<T, D, Metric>::SearchDataExtras::AxisData &current_axis = search_data.axis[axis];
  return current_distance += (split_value - current_axis.nearest) * (current_axis.nearest + split_value - current_axis.p * 2.0);
}

/**
 * \brief Incrementally update the distance from the implicit reference vector p to the current hyperrectangle using the Euclidean metric.
 * Specialization for non-fundamental types that avoids unnecessary copies of objects.
 *
 * See the documentation for the template parameter \c isFundamental = \c true specialization for details.
 */
template <typename T, const unsigned int D>
T& EuclideanIncrementalFunctor<T, D, false>::operator () (T &current_distance, unsigned int axis, ConstRef_T split_value, const KDSearchData<T, D, Metric> &search_data) const {
  const typename IncrementalBase<T, D, Metric>::SearchDataExtras::AxisData &current_axis = search_data.axis[axis];
  T acc1 = split_value;
  acc1 -= current_axis.nearest;
  T acc2 = split_value;
  acc2 += current_axis.nearest;
  acc2 -= current_axis.p;
  acc2 -= current_axis.p;
  acc1 *= acc2;
  return current_distance += acc1;
}

/// Update the current incremental distance using the Euclidean metric.
template <typename T, const unsigned int D>
EuclideanIncrementalUpdater<T, D>::EuclideanIncrementalUpdater(const KDNode<T, D> *node, const KDNode<T, D> *parent, KDSearchData<T, D, Metric> &search_data)
    : IncrementalBase<T, D, Metric>(search_data) {
  IncrementalBase<T, D, Metric>::update(node, parent, search_data, EuclideanIncrementalFunctor<T, D>());
}

/// Undo any incremental updates performed to the hyperrectangle distance.
template <typename T, const unsigned int D>
EuclideanIncrementalUpdater<T, D>::~EuclideanIncrementalUpdater() {
  IncrementalBase<T, D, Metric>::restore();
}

} // namespace kche_tree
