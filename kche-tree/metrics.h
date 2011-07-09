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

#include "kche-tree.h"
#include "kd-node.h"
#include "incremental.h"
#include "vector.h"

namespace kche_tree {

/**
 * \brief Template providing Euclidean metrics.
 *
 * Provides Euclidean distance metrics to any pair of same-length feature vectors.
 * It also specifies the type providing incremental hyperrectangle distance calculation for this metric.
 *
 * \tparam T Type of the elements the metric is applied to. The +, +=, -, * and > operators are required.
 * \tparam D Number of dimensions of the vectors the metric is applied to.
 */
template <typename T, const unsigned int D>
class EuclideanMetric {
public:
  /// Type of the elements to which the metrics are applied.
  typedef T ElementType;

  /// Number of dimensions to which the metrics are applied.
  static unsigned const int Dimensions = D;

  /// Type for incremental hyperrectangle intersection calculations when using this metric.
  typedef EuclideanIncremental<T, D> IncrementalType;

  /// Use the global vector type by default.
  typedef typename Settings<T, D>::VectorType VectorType;

  /// Squared distance to a feature vector.
  inline T operator () (const VectorType &v1, const VectorType &v2) const;

  /// Squared distance to a feature vector with an upper bound.
  inline T operator () (const VectorType &v1, const VectorType &v2, const T &upper_bound) const;
};

} // namespace kche_tree

// Template implementation.
#include "metrics.tpp"

#endif
