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
 * \file kd-search.tpp
 * \brief Template iplementation for the data used when searching the kd-tree.
 * \author Leandro Graciá Gil
 */

namespace kche_tree {

/**
 * Initialize a data searching structure with incremental hyperrectangle intersection calculation.
 *
 * \param p Reference point being used in the search.
 * \param data Permuted training set stored by the tree.
 * \param metric Metric functor used for the distance calculations.
 * \param K Number of neighbours to retrieve.
 * \param ignore_null_distances_arg Indicate that points with null distance should be ignored.
 */
template <typename T, unsigned int D, typename M>
KDSearch<T, D, M>::KDSearch(const Vector &p, const DataSet &data, const M &metric, unsigned int K, bool ignore_null_distances_arg)
  : M::IncrementalUpdater::SearchExtras(p, data),
    p(p),
    data(data),
    metric(metric),
    K(K),
    hyperrect_distance(Traits<Distance>::zero()),
    farthest_distance(Traits<Distance>::zero()),
    ignore_null_distances(ignore_null_distances_arg) {}

} // namespace kche_tree
