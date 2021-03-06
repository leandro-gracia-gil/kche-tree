/***************************************************************************
 *   Copyright (C) 2011, 2012 by Leandro Graciá Gil                        *
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
#include "utils.h"
#include "vector.h"

namespace kche_tree {

// Forward declarations.
template <typename T, unsigned int D, typename M> class KDSearch;
template <typename T, unsigned int D> class EuclideanMetric;
template <typename T, unsigned int D> class MahalanobisMetric;

/**
 * \brief Provide the basic operations for an axis-based incremental hyperrectangle distance calculation.
 *
 * This class takes care of all the details of axis-based incremental distance calculations
 * with the exception of the actual value update, which is left to the corresponding functor.
 *
 * \tparam T Type of the elements used. The operators <, > and = are required for this type.
 * \tparam D Number of dimensions in the vectors.
 * \tparam Metric Metric type associated with the incremental calculation.
 */
template <typename ElementType, unsigned int NumDimensions, typename MetricType>
class IncrementalBase {
public:
  /// Type of the elements used in the incremental calculation.
  typedef ElementType Element;

  /// Number of dimensions of the feature vectors.
  static const unsigned int Dimensions = NumDimensions;

  // Type of the metric associated with the incremental calculation.
  typedef MetricType Metric;

  /// Alias for compatible non-labeled data sets.
  typedef typename kche_tree::DataSet<Element, Dimensions> DataSet;

  /// Alias for the compatible feature vectors.
  typedef typename kche_tree::Vector<Element, Dimensions> Vector;

  /// Alias for the compatible kd-tree search structure.
  typedef typename kche_tree::KDSearch<Element, Dimensions, Metric> KDSearch;

  /// Alias for the compatible kd-nodes.
  typedef typename kche_tree::KDNode<Element, Dimensions> KDNode;

  /// Alias for the associated distance type.
  typedef typename Traits<Element>::Distance Distance;

  /// Extra data required in the KDSearch struct to be able to perform axis-based incremental hyperrectangle intersection calculations.
  struct SearchData {

    /// Axis-ordered point and hyperrectangle structure. Used internally to increase the cache hits.
    struct AxisData {
      Element p; ///< Per-axis reference input point.
      Element nearest; ///< Per-axis nearest point in the current hyperrectangle.
    } axis[Dimensions]; ///< Per-axis data defined this way to reduce cache misses.

    /// Fill per-axis data contents.
    SearchData(const Vector &p, const DataSet &data);
  };

  /// Type of the data extension applied to the KDSearch object for incremental calculations.
  typedef SearchData SearchExtras;

  /// Default constructor.
  IncrementalBase(KDSearch &search_data);

  /// Create an temporary incremental calculator object. Will update the current data according with the selected branch.
  template <typename IncrementalFunctor>
  void update(const KDNode *node, const KDNode *parent, KDSearch &search_data, const IncrementalFunctor &updater);

  /// Undo any previous modifications to the incremental data.
  void restore();

protected:
  KDSearch &search_data_; ///< Reference to the search data being used.
  unsigned int parent_axis_; ///< Axis that defines the hyperspace splitting.

  bool modified_; ///< Flag indicating if the values were modified as part of the incremental update.
  Element previous_axis_nearest_; ///< Previous value of the local axis in the hyperrectangle.
  Distance previous_hyperrect_distance_; ///< Previous value of the distance to the nearest point in the hyperrectangle.
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

  EuclideanIncrementalUpdater(const KDNode<T, D> *node, const KDNode<T, D> *parent, KDSearch<T, D, Metric> &search_data);
  ~EuclideanIncrementalUpdater();
};

/**
 * \brief Perform an incremental hyperrectangle distance update based on the Mahalanobis metric.
 * The cost of the update will be considerably reduced if the inverse covariance matrix associated with the metric object used is diagonal.
 *
 * \tparam T Type of the elements used. The operators +=, -= and *= are required for this type in addition to the ones required by IncrementalBase.
 * \tparam D Number of dimensions in the vectors.
 */
template <typename T, const unsigned int D>
class MahalanobisIncrementalUpdater : public IncrementalBase<T, D, MahalanobisMetric<T, D> > {
public:
  /// Metric associated with this incremental calculation.
  typedef MahalanobisMetric<T, D> Metric;

  MahalanobisIncrementalUpdater(const KDNode<T, D> *node, const KDNode<T, D> *parent, KDSearch<T, D, Metric> &search_data);
  ~MahalanobisIncrementalUpdater();
};


} // namespace kche_tree

// Template implementation files.
#include "incremental.tpp"
#include "incremental_euclidean.tpp"
#include "incremental_mahalanobis.tpp"

#endif
