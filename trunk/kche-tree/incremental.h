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
 * \file incremental.h
 * \brief Template definitions for incremental hyperrectangle distance operators.
 * \author Leandro Graciá Gil
*/

#ifndef _KCHE_TREE_INCREMENTAL_H_
#define _KCHE_TREE_INCREMENTAL_H_

#include "dataset.h"
#include "kd-node.h"
#include "rparam.h"
#include "vector.h"

namespace kche_tree {

// Forward declarations.
template <typename T, const unsigned int D, typename M> class KDSearchData;
template <typename T, const unsigned int D> class EuclideanMetric;
template <typename T, const unsigned int D, bool isFundamental = IsFundamental<T>::value> struct EuclideanIncrementalFunctor;

/**
 * \brief Provide the basic operations for an axis-based incremental hyperrectangle distance calculation.
 *
 * This class takes care of all the details of incremental distance calculation with the exception of the actual value update,
 * which is left to the provided functor.
 *
 * \tparam T Type of the elements used. The operators <, > and = are required for this type.
 * \tparam D Number of dimensions in the vectors.
 * \tparam Metric Metric type associated with the incremental calculation.
 */
template <typename T, const unsigned int D, typename Metric>
class IncrementalBase {
public:
  /// Use the global data set type by default.
  typedef typename Settings<T, D>::DataSetType DataSetType;

  /// Use the global vector type by default.
  typedef typename Settings<T, D>::VectorType VectorType;

  /// Extra data required in the KDSearchData struct to be able to perform axis-based incremental hyperrectangle intersection calculations.
  struct SearchData {

    /// Axis-ordered point and hyperrectangle structure. Used internally to increase the cache hits.
    struct AxisData {
      T p; ///< Per-axis reference input point.
      T nearest; ///< Per-axis nearest point in the current hyperrectangle.
    } axis[D]; ///< Per-axis data defined this way to reduce cache misses.

    /// Fill per-axis data contents.
    SearchData(const VectorType &p, const DataSetType &data);
  };

  /// Type of the data extension applied to the KDSearchData object for incremental calculations.
  typedef SearchData SearchDataExtras;

  /// Default constructor.
  IncrementalBase(KDSearchData<T, D, Metric> &search_data);

  /// Create an temporary incremental calculator object. Will update the current data according with the selected branch.
  template <typename IncrementalFunctor>
  void update(const KDNode<T, D> *node, const KDNode<T, D> *parent, KDSearchData<T, D, Metric> &search_data, const IncrementalFunctor &updater);

  /// Undo any previous modifications to the incremental data.
  void restore();

protected:
  KDSearchData<T, D, Metric> &search_data_; ///< Reference to the search data being used.
  unsigned int parent_axis_; ///< Axis that defines the hyperspace splitting.

  bool modified_; ///< Flag indicating if the values were modified as part of the incremental update.
  T previous_axis_nearest_; ///< Previous value of the local axis in the hyperrectangle.
  T previous_hyperrect_distance_; ///< Previous value of the distance to the nearest point in the hyperrectangle.
};

/**
 * \brief Perform an incremental hyperrectangle distance update based on the Euclidean metric.
 *
 * \tparam T Type of the elements used. The operators +=, -= and *= are required for this type in addition to the ones required by IncrementalBase.
 * \tparam D Number of dimensions in the vectors.
 */
template <typename T, const unsigned int D>
class EuclideanIncrementalUpdater : public IncrementalBase<T, D, EuclideanMetric<T, D> > {
public:
  /// Metric associated with this incremental calculation.
  typedef EuclideanMetric<T, D> Metric;

  /// Functor providing the incremental calculation.
  typedef EuclideanIncrementalFunctor<T, D> IncrementalFunctor;

  /// Update the current incremental distance using a provided functor.
  EuclideanIncrementalUpdater(const KDNode<T, D> *node, const KDNode<T, D> *parent, KDSearchData<T, D, Metric> &search_data)
    : IncrementalBase<T, D, Metric>(search_data) { IncrementalBase<T, D, Metric>::update(node, parent, search_data, IncrementalFunctor()); }

  /// Undo any incremental updates performed to the hyperrectangle distance.
  ~EuclideanIncrementalUpdater() {
    IncrementalBase<T, D, Metric>::restore();
  }
};

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

  /// Type of the per-axis data provided to the functor.
  typedef typename IncrementalBase<T, D, Metric>::SearchDataExtras::AxisData AxisType;

  /**
   * \brief Incrementally update the distance from the implicit reference vector p to the current hyperrectangle.
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
   * \param current_distance Current distance to the hyperrectangle. Will be updated with the new distance.
   * \param axis Number of the axis (dimension) being updated.
   * \param split_value Split value used to halve the hyperspace across the \a axis dimension.
   * \param axis_data Current nearest to the hyperrectangle and reference values for the \a axis dimension.
   * \param search_data KD-tree search data information. Currently not required by the Euclidean metric.
   */
  T& operator () (T &current_distance, unsigned int axis, ConstRef_T split_value,
      const AxisType *axis_data, const KDSearchData<T, D, Metric> &search_data) const;
};

/// Especialization for non-fundamental types. Makes use only of the +=, -= and *= operators.
template <typename T, const unsigned int D>
struct EuclideanIncrementalFunctor<T, D, false> {

  /// Metric associated with this incremental calculation.
  typedef EuclideanMetric<T, D> Metric;

  /// Use optimized const reference types.
  typedef typename RParam<T>::Type ConstRef_T;

  /// Type of the per-axis data provided to the functor.
  typedef typename IncrementalBase<T, D, Metric>::SearchDataExtras::AxisData AxisType;

  /**
   * \brief Incrementally update the distance from the implicit reference vector p to the current hyperrectangle.
   *
   * See the documentation for the template parameter \c isFundamental = \c true specialization for details.
   */
  T& operator () (T &current_distance, unsigned int axis, ConstRef_T split_value,
      const AxisType *axis_data, const KDSearchData<T, D, Metric> &search_data) const;
};

} // namespace kche_tree

// Template implementation.
#include "incremental.tpp"

#endif
