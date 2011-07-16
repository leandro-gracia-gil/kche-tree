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
 * \file incremental.tpp
 * \brief Template implementations for incremental hyperrectangle intersection operators.
 * \author Leandro Graciá Gil
 */

namespace kche_tree {

/**
 * Initialize the axis-specific data to the reference point.
 *
 * \param p Reference point.
 * \param data Input data. Unused in this case.
 */
template <typename T, const unsigned int D, typename M>
IncrementalBase<T, D, M>::SearchData::SearchData(const VectorType &p, const DataSetType &data) {
  for (unsigned int d=0; d<D; ++d) {
    axis[d].p = p[d];
    axis[d].nearest = p[d];
  }
}

/**
 * Default constructor. Set the reference to the KD-tree search data structure.
 *
 * \param search_data Data structure used for tree traversal and incremental calculations.
 */
template <typename T, const unsigned int D, typename Metric>
IncrementalBase<T, D, Metric>::IncrementalBase(KDSearchData<T, D, Metric> &search_data)
  : search_data_(search_data),
    parent_axis_(0),
    modified_(false),
    previous_axis_nearest_(Traits<T>::zero()),
    previous_hyperrect_distance_(Traits<T>::zero()) { }

/**
 * Perform an incremental update of the distance to the nearest point in the hyperrectangle.
 *
 * \tparam Functor used to update the distance to the hyperrectangle.
 * \param node Current node in the sub-hyperrectangular region.
 * \param parent Parent node that halves the hyperspace in two.
 * \param search_data Auxiliar data structure used for tree traversal and incremental calculations.
 * \param updater Functor object used to update the current hyperrectangle distance.
 */
template <typename T, const unsigned int D, typename Metric> template <typename IncrementalFunctor>
void IncrementalBase<T, D, Metric>::update(const KDNode<T, D> *node, const KDNode<T, D> *parent, KDSearchData<T, D, Metric> &search_data, const IncrementalFunctor &updater) {

  // Check parent.
  if (parent == NULL)
    return;

  // Get splitting axis data.
  parent_axis_ = parent->axis & KDNode<T, D>::axis_mask;
  typename SearchDataExtras::AxisData *axis_data = &search_data.axis[parent_axis_];

  // Check if current branch modifies the bounding hyperrectangle.
  if ((parent->left_branch  == node && parent->split_value > axis_data->nearest) ||
      (parent->right_branch == node && parent->split_value < axis_data->nearest))
    return;

  // Store current values before any update.
  modified_ = true;
  previous_axis_nearest_ = axis_data->nearest;
  previous_hyperrect_distance_ = search_data.hyperrect_distance;

  // Calculate the new distance to the hyperrectangle.
  updater(search_data_.hyperrect_distance, parent_axis_, parent->split_value, axis_data, search_data);

  // Define the new boundaries of the hyperrectangle.
  axis_data->nearest = parent->split_value;
}

/**
 * Restore the updated values to their previous ones, if modified.
 */
template <typename T, const unsigned int D, typename Metric>
void IncrementalBase<T, D, Metric>::restore() {

  // Restore previous values if modified.
  if (modified_) {
    search_data_.axis[parent_axis_].nearest = previous_axis_nearest_;
    search_data_.hyperrect_distance = previous_hyperrect_distance_;
  }
}

/**
 * \brief Incrementally update the distance from the implicit reference vector p to the current hyperrectangle.

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
 * \param axis_data Current nearest to the hyperrectangle and reference values for the \a axis dimension.
 * \param search_data KD-tree search data information. Currently not required by the Euclidean metric.
 * \return The reference to the current distance to the hyperrectangle. Should have been updated.
 */
template <typename T, const unsigned int D>
T& EuclideanIncrementalFunctor<T, D, true>::operator () (T &current_distance, unsigned int axis, ConstRef_T split_value, const AxisType *axis_data, const KDSearchData<T, D, Metric> &search_data) const {
  return current_distance += (split_value - axis_data->nearest) * (axis_data->nearest + split_value - axis_data->p - axis_data->p);
}

/**
 * \brief Incrementally update the distance from the implicit reference vector p to the current hyperrectangle.
 *
 * See the documentation for the template parameter \c isFundamental = \c true specialization for details.
 */
template <typename T, const unsigned int D>
T& EuclideanIncrementalFunctor<T, D, false>::operator () (T &current_distance, unsigned int axis, ConstRef_T split_value, const AxisType *axis_data, const KDSearchData<T, D, Metric> &search_data) const {
  T acc1 = split_value;
  acc1 -= axis_data->nearest;
  T acc2 = split_value;
  acc2 += axis_data->nearest;
  acc2 -= axis_data->p;
  acc2 -= axis_data->p;
  acc1 *= acc2;
  return current_distance += acc1;
}

} // namespace kche_tree
