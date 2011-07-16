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

template <typename T, bool isFundamental = IsFundamental<T>::value> struct DotFunctor;

/// Dot product functor especialization for fundamental types.
template <typename T>
struct DotFunctor<T, true> {
  /**
   * \brief Functor to calculate the dot product from the difference in a given dimension between in 2 arrays.
   *
   * \tparam Type used for the accumulator.
   * \param acc Accumulator to be updated.
   * \param a First array.
   * \param b Second array.
   * \param i Index of the dimension being calculated.
   */
  template <typename TAcc>
  T& operator () (TAcc &acc, const T *a, const T *b, unsigned int i) const {
    return acc += (a[i] - b[i]) * (a[i] - b[i]);
  }
};

/**
 * \brief Dot product functor especialization for non-fundamental types.
 *
 * Makes use of the =, -= and *= operators for the \a T type and the += operator for the accumulator type.
 */
template <typename T>
struct DotFunctor<T, false> {
  /**
   * \brief Functor to calculate the dot product from the difference in a given dimension between in 2 arrays.
   *
   * \tparam Type used for the accumulator.
   * \param acc Accumulator to be updated.
   * \param a First array.
   * \param b Second array.
   * \param i Index of the dimension being calculated.
   */
  template <typename TAcc>
  T& operator () (TAcc &acc, const T *a, const T *b, unsigned int i) const {
    T temp = a[i];
    temp -= b[i];
    temp *= temp;
    return acc += temp;
  }
};

/**
 * Generic squared euclidean distance operator for two D-dimensional numeric feature vectors.
 * Redefinitions and specializations of this operator are welcome.
 *
 * \param v1 First feature vector.
 * \param v2 Second feature vector.
 * \return Euclidean squared distance between the two vectors.
 */
template <typename T, const unsigned int D>
T EuclideanMetric<T, D>::operator () (const VectorType &v1, const VectorType &v2) const {

  // Standard squared distance between two D-dimensional vectors.
  typename Traits<T>::AccumulatorType acc = Traits<T>::zero();
  MapReduce<T, D>::run(DotFunctor<T>(), v1.data(), v2.data(), acc);
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
T EuclideanMetric<T, D>::operator () (const VectorType &v1, const VectorType &v2, ConstRef_T upper_bound) const {

  // Constant calculated empirically.
  const unsigned int D_acc = (unsigned int) (0.4f * D);

  // Accumulate the first D_acc dimensions without any kind of check.
  typename Traits<T>::AccumulatorType acc = Traits<T>::zero();
  MapReduce<T, D_acc>::run(DotFunctor<T>(), v1.data(), v2.data(), acc);

  // Calculate the remaining dimensions using an upper bound, and checking it every 4 dimensions.
  // The template metaprogramming makes sure this interval is performed without actually checking any index or iterator at runtime.
  // Has been tested to be faster than a loop with the difference being more acute with greater D values.
  BoundedMapReduce<T, D, 4, D_acc>::run(DotFunctor<T>(), std::greater<T>(), v1.data(), v2.data(), upper_bound, acc);
  return acc;
}

} // namespace kche_tree
