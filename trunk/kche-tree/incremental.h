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
 * \tparam T Type of the elements used. The operators +, - and * are required for this type in addition to the ones required by IncrementalBase.
 * \tparam D Number of dimensions in the vectors.
 */
template <typename T, const unsigned int D>
class EuclideanIncremental : public IncrementalBase<T, D, EuclideanMetric<T, D> > {
public:
  /// Metric associated with this incremental calculation.
  typedef EuclideanMetric<T, D> Metric;

  /// Use optimized const reference types.
  typedef typename RParam<T>::Type ConstRef_T;

  /// Euclidean hyperrectangle distance incremental functor.
  struct IncrementalFunctor {
    T operator () (ConstRef_T current_distance, unsigned int axis, ConstRef_T split_value,
        const typename IncrementalBase<T, D, Metric>::SearchDataExtras::AxisData *axis_data, const KDSearchData<T, D, Metric> &search_data) const;
  };

  /// Encapsulate the use of AxisIncremental with its own incremental distance functor.
  EuclideanIncremental(const KDNode<T, D> *node, const KDNode<T, D> *parent, KDSearchData<T, D, Metric> &search_data)
    : IncrementalBase<T, D, Metric>(search_data) { IncrementalBase<T, D, Metric>::update(node, parent, search_data, IncrementalFunctor()); }

  ~EuclideanIncremental() {
    IncrementalBase<T, D, Metric>::restore();
  }
};

} // namespace kche_tree

// Template implementation.
#include "incremental.tpp"

#endif
