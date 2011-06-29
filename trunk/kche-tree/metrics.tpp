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
 * \file metrics.tpp
 * \brief Template implementations for metric functors.
 * \author Leandro Graciá Gil
 */

// Include the map-reduce metaprograming templates and traits.
#include "map_reduce.h"
#include "traits.h"

namespace kche_tree {

/**
 * \brief Functor to calculate the dot product of the difference between elements in 2 arrays.
 *
 * \param a First array.
 * \param b Second array.
 * \param i Index of the dimension being calculated.
 * \return The dot product of the difference between the elements in the \a i-th dimension.
 */
template <typename T>
struct DotFunctor {
  T operator () (const T *a, const T *b, unsigned int i) const {
    return (a[i] - b[i]) * (a[i] - b[i]);
  }
};

/**
 * Generic squared euclidean distance operator for two D-dimensional numeric feature vectors.
 * Redefinitions and specializations of this operator are welcome.
 *
 * \param v1 First feature vector.
 * \param v1 Second feature vector.
 * \return Euclidean squared distance between the two vectors.
 */
template <typename T, const unsigned int D>
T EuclideanMetric<T, D>::operator () (const Vector<T, D> &v1, const Vector<T, D> &v2) const {

  // Standard squared distance between two D-dimensional vectors.
  DotFunctor<T> dot;
  typename Traits<T>::AccumulatorType acc = Traits<T>::zero();
  for (unsigned int i=0; i<D; ++i)
    acc += dot(v1.data(), v2.data(), i);
  return acc;
}

/**
 * Generic squared euclidean distance operator for two D-dimensional numeric feature vectors.
 * Special version with early leaving in case an upper bound value is reached.
 *
 * \param v1 Frist feature vector.
 * \param v2 Second feature vector.
 * \param upper_bound Upper bound for the distance. Will return immediatly if reached.
 * \return Euclidean squared distance between the two vectors or a partial result greater than \a upper.
 */
template <typename T, const unsigned int D>
T EuclideanMetric<T, D>::operator () (const Vector<T, D> &v1, const Vector<T, D> &v2, const T &upper_bound) const {

  // Constant calculated empirically.
  const unsigned int D_acc = (unsigned int) (0.4f * D);

  // This has been empirically compared with the MapReduce template metaprogramming class,
  // but the loop seemed to be always faster because of the code locality.
  DotFunctor<T> dot;
  typename Traits<T>::AccumulatorType acc = Traits<T>::zero();
  for (unsigned int i=0; i<D_acc; ++i)
    acc += dot(v1.data(), v2.data(), i);

  // Calculate the remaining dimensions using an upper bound, and checking it every 4 dimensions.
  // The template metaprogramming makes sure this interval is performed without actually checking any index or iterator at runtime.
  // Has been tested to be faster than a loop with the difference being more acute with greater D values.
  return BoundedMapReduce<T, D, 4, D_acc>::run(DotFunctor<T>(), std::plus<T>(), std::greater<T>(), v1.data(), v2.data(), upper_bound, acc);
}

} // namespace kche_tree
