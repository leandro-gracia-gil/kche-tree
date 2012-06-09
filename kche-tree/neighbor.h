/***************************************************************************
 *   Copyright (C) 2010, 2011, 2012 by Leandro Graciá Gil                  *
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
 * \file neighbor.h
 * \brief Template for indexed neighbours and their distances.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_NEIGHBOUR_H_
#define _KCHE_TREE_NEIGHBOUR_H_

#include "utils.h"

namespace kche_tree {

/**
 * \brief References a feature vector by its index in the data set and provides the squared distance to it from an implicit vector.
 *
 * Implements its own comparison function with the parenthesis operator for STL-based algorithm use.
 *
 * \tparam Distance Type used to encode the distance between two feature vectors.
 */
template <typename Distance>
class Neighbor {
public:
  /// Use optimized const reference types.
  typedef typename RParam<Distance>::Type ConstRef_Distance;

  /// Value used to define invalid neighbour indices. Used when initializing empty objects.
  static const unsigned int invalid_index = -1;

  // Default and convenience constructors.
  Neighbor() : index_(invalid_index), squared_distance_(Distance()) {}
  Neighbor(unsigned int index, ConstRef_Distance squared_distance) : index_(index), squared_distance_(squared_distance) {}

  unsigned int index() const { return index_; }
  ConstRef_Distance squared_distance() const { return squared_distance_; }

  /// Distance comparison operator for Neighbors. Allows Neighbor objects to be used as STL comparison functors.
  struct DistanceComparer : public std::binary_function<Neighbor<Distance>, Neighbor<Distance>, bool> {
    bool operator () (const Neighbor &v1, const Neighbor &v2) const {
      return v1.squared_distance() < v2.squared_distance();
    }
  };

private:
  // KD-trees can update the indices.
  template <typename T, unsigned int D, typename Label> friend class KDTree;
  void set_index(unsigned int new_index) { index_ = new_index; }

  unsigned int index_; ///< Index of the feature vector in the data set.
  Distance squared_distance_; ///< Squared distance of the referenced element to an implicit vector.
};

} // namespace kche_tree

#endif
