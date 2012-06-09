/***************************************************************************
 *   Copyright (C) 2012 by Leandro Graciá Gil                              *
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
 * \file kd-search.h
 * \brief Template for data used when searching the kd-tree.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_KD_SEARCH_H_
#define _KCHE_TREE_KD_SEARCH_H_

namespace kche_tree {

/**
 * \brief Structure holding the data specific to search in the tree.
 *
 * Will be expanded with any extra data provided by the associated incremental calculation type.
 */
template <typename ElementType, unsigned int NumDimensions, typename MetricType>
struct KDSearch : MetricType::IncrementalUpdater::SearchExtras {

  /// Type of the elements used in the incremental calculation.
  typedef ElementType Element;

  /// Number of dimensions of the feature vectors.
  static const unsigned int Dimensions = NumDimensions;

  // Type of the metric associated with the incremental calculation.
  typedef MetricType Metric;

  /// Distance type associated with the elements.
  typedef typename Traits<Element>::Distance Distance;

  // Alias of compatible feature vectors.
  typedef typename kche_tree::Vector<Element, Dimensions> Vector;

  /// Alias of compatible non-labeled data sets.
  typedef typename kche_tree::DataSet<Element, Dimensions> DataSet;

  const Vector &p; ///< Reference input point.
  const DataSet &data; ///< Permuted training set.
  const Metric &metric; ///< Metric functor used to calculate distances between points.
  unsigned int K; ///< Number of neighbours to retrieve.

  Distance hyperrect_distance; ///< Distance to the current nearest point in the hyperrectangle.
  Distance farthest_distance; ///< Current distance from the farthest nearest neighbour to the reference point.
  bool ignore_null_distances;  ///< Used to exclude the source point if it's already in the tree.

  /// Initialize data for a tree search with incremental intersection calculation.
  KDSearch(const Vector &p, const DataSet &data, const Metric &metric, unsigned int K, bool ignore_p_in_tree);
};

} // namespace kche_tree

// Template implementation.
#include "kd-search.tpp"

#endif
