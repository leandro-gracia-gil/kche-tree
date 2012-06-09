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
 * \file metrics.h
 * \brief Template definitions for metric functors used to calculate distances.
 * \author Leandro Graciá Gil
 *
 * \warning The following is assumed for any distance metric.\n\n
 * Let d: X * X -> R where R is the set of real numbers, for any x, y, z in X: \n
 * 1. d(x, y) >= 0  (non-negativity). \n
 * 2. d(x, y) = 0 if and only if x = y (identity of indiscernibles). \n
 * 3. d(x, y) = d(y, x)  (symmetry). \n
 * 4. d(x, z) <= d(x, y) + d(y, z)  (triangle inequality). \n\n
 * Please make sure this is true for any metric not provided by the library.
 */

#ifndef _KCHE_TREE_METRICS_H_
#define _KCHE_TREE_METRICS_H_

#include "incremental.h"
#include "kd-node.h"
#include "symmetric_matrix.h"
#include "utils.h"
#include "vector.h"

namespace kche_tree {

/**
 * \brief Template providing Euclidean metrics.
 *
 * Provides Euclidean distance metrics to any pair of same-length feature vectors.
 * It also specifies the type providing incremental hyperrectangle distance calculation for this metric.
 *
 * \tparam T Type of the elements the metric is applied to. The +=, *=, -= and > operators are required.
 * \tparam D Number of dimensions of the vectors the metric is applied to.
 */
template <typename T, const unsigned int D>
class EuclideanMetric {
public:
  /// Type of the elements to which the metrics are applied.
  typedef T Element;

  /// Number of dimensions to which the metrics are applied.
  static unsigned const int Dimensions = D;

  /// Type for incremental hyperrectangle intersection calculations when using this metric.
  typedef EuclideanIncrementalUpdater<T, D> IncrementalUpdater;

  /// Alias for the associated distance type.
  typedef typename Traits<Element>::Distance Distance;

  /// Alias for the compatible feature vectors.
  typedef typename kche_tree::Vector<Element, Dimensions> Vector;

  /// Use optimized const reference type for distance.
  typedef typename RParam<Distance>::Type ConstRef_Distance;

  // Squared distance to a feature vector.
  inline Distance operator () (const Vector &v1, const Vector &v2) const;

  // Squared distance to a feature vector with an upper bound.
  inline Distance operator () (const Vector &v1, const Vector &v2, ConstRef_Distance upper_boundary) const;
};

/**
 * \brief Template providing the Mahalanobis metric.
 *
 * Provides Mahalanobis distance metrics to any pair of same-length feature vectors.
 * It also specifies the type providing incremental hyperrectangle distance calculation for this metric.
 *
 * Requires *= (float) plus requirements for symmetric matrix inversion.
 * \note This metric assumes \a T to be commutative under multiplication and distributive over addition.
 *
 * \tparam T Type of the elements the metric is applied to. The +=, -=, *= and > operators are required.
 * \tparam D Number of dimensions of the vectors the metric is applied to.
 */
template <typename T, const unsigned int D>
class MahalanobisMetric {
public:
  /// Type of the elements to which the metrics are applied.
  typedef T Element;

  /// Number of dimensions to which the metrics are applied.
  static unsigned const int Dimensions = D;

  /// Type for incremental hyperrectangle intersection calculations when using this metric.
  typedef MahalanobisIncrementalUpdater<T, D> IncrementalUpdater;

  /// Alias for the associated distance type.
  typedef typename Traits<Element>::Distance Distance;

  /// Alias for the compatible feature vectors.
  typedef typename kche_tree::Vector<Element, Dimensions> Vector;

  /// Alias for compatible non-labeled data sets.
  typedef typename kche_tree::DataSet<Element, Dimensions> DataSet;

  /// Use optimized const reference type for distance.
  typedef typename RParam<Distance>::Type ConstRef_Distance;

  /// Type of the vector used to hold partial results while evaluating the metric.
  typedef kche_tree::Vector<Distance, KCHE_TREE_SSE_COMPILE_ALIGN(Distance, D)> CacheVector;

  // Constructors.
  MahalanobisMetric();
  MahalanobisMetric(const DataSet &train_set);

  // Inverse covariance matrix manipulation.
  // Matrix is assumed to have the properties of the inverse of a covariance matrix
  // (symmetric positive-definite, as inverting will fail for non-invertible positive-semidefinite ones).
  bool set_inverse_covariance(const DataSet &train_set);
  bool set_inverse_covariance(const Distance *inverse_covariance);
  bool set_diagonal_covariance(const Distance *diagonal);
  void force_diagonal_covariance();

  const SymmetricMatrix<Distance> &inverse_covariance() const { return inv_covariance_; } ///< Retrieve the inverse covariance matrix associated to the metric.
  bool has_diagonal_covariance() const { return is_diagonal_; } ///< Check if the inverse covariance matrix is diagonal.

  // Squared distance to a feature vector.
  inline Distance operator () (const Vector &v1, const Vector &v2) const;

  // Squared distance to a feature vector with an upper bound.
  inline Distance operator () (const Vector &v1, const Vector &v2, ConstRef_Distance upper_boundary) const;

private:
  SymmetricMatrix<Distance> inv_covariance_; ///< Inverse covariance matrix associated with the metric instance.
  bool is_diagonal_; ///< Flag indicating if the inverse covariance matrix is diagonal and hence enabling severe optimizations.
};

} // namespace kche_tree

// Template implementation files.
#include "metrics_euclidean.tpp"
#include "metrics_mahalanobis.tpp"

#endif
