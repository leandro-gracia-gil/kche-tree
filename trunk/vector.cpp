/***************************************************************************
 *   Copyright (C) 2010, 2011 by Leandro Graciá Gil                        *
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
 * \file vector.cpp
 * \brief Template implementations for generic D-dimensional feature vectors.
 * \author Leandro Graciá Gil
 */

// Include the map-reduce metaprograming templates.
#include "mapreduce.h"

namespace kche_tree {

/**
 * Functor to calculate the squared difference between elements in 2 arrays.
 *
 * \param a First array.
 * \param b Second array.
 * \param i Index of the dimension being calculated.
 * \return The squared difference between the elements in the \a i-th dimension.
 */
template <typename T>
struct DotFunctor {
  T operator () (const T *a, const T *b, unsigned int i) const {
    return (a[i] - b[i]) * (a[i] - b[i]);
  }
};


/**
 * Generic memory allocator operator for feature vector arrays.
 * Defined so that memory-aligned specializations can be defined if required.
 *
 * \param size Total size in bytes of the objects to allocate.
 * \return Address to the new allocated memory.
 */
template <typename T, const unsigned int D>
void *Vector<T, D>::operator new [] (size_t size) {

  // Allocate plain memory for the requested vectors.
  void *p = malloc(size);

  // Throw an allocation exception in case of error.
  if (p == NULL) {
    std::bad_alloc exception;
    throw exception;
  }

  return p;
}

/**
 * Generic memory deallocator operator for feature vector arrays.
 * Defined as complement of operator new [] so that memory-aligned specializations can be defined if required.
 *
 * \param p Pointer to the address to release.
 */
template <typename T, const unsigned int D>
void Vector<T, D>::operator delete [] (void *p) {

  // Just release memory.
  free(p);
}

/**
 * Check if two feature vectors are exactly equal.
 *
 * \param p Feature vector being compared to.
 * \return \c true if equal, \c false otherwise.
 */
template <typename T, const unsigned int D>
bool Vector<T, D>::operator == (const Vector &p) const {
  return equal_arrays(data, p.data, D);
}

/**
 * Check if two feature vectors are different equal.
 *
 * \param p Feature vector being compared to.
 * \return \c true if different, \c false otherwise.
 */
template <typename T, const unsigned int D>
bool Vector<T, D>::operator != (const Vector &p) const {
  return !equal_arrays(data, p.data, D);
}

/**
 * Generic squared euclidean distance operator for two D-dimensional numeric kd_points.
 * Redefinitions and specializations of this operator are welcome.
 *
 * \param p Point being compared to.
 * \return Euclidean squared distance between the two kd_points.
 */
template <typename T, const unsigned int D>
T Vector<T, D>::distance_to(const Vector &p) const {

  // Standard squared distance between two D-dimensional vectors.
  T acc = 0;
  for (unsigned int i=0; i<D; ++i)
    acc += (data[i] - p.data[i]) * (data[i] - p.data[i]);
  return acc;
}

/**
 * Generic squared euclidean distance operator for two D-dimensional numeric kd_points.
 * Special version with early leaving in case an upper bound value is reached.
 *
 * \param p Point being compared to.
 * \param upper_bound Upper bound for the distance. Will return immediatly if reached.
 * \return Euclidean squared distance between the two kd_points or a partial result greater or equal than \a upper.
 */
template <typename T, const unsigned int D>
T Vector<T, D>::distance_to(const Vector &p, T upper_bound) const {

  // Constant calculated empirically.
  const unsigned int D_acc = (unsigned int) (0.4f * D);

  // This has been empirically compared with the MapReduce template metaprogramming class,
  // but the loop seemed to be always faster because of the code locality.
  T acc = 0;
  for (unsigned int i=0; i<D_acc; ++i)
    acc += (data[i] - p.data[i]) * (data[i] - p.data[i]);

  // Calculate the remaining dimensions using an upper bound, and checking it every 4 dimensions.
  // The template metaprogramming makes sure this interval is performed without actually checking any index or iterator at runtime.
  // Has been tested to be faster than a loop with the difference being more acute with greater D values.
  return BoundedMapReduce<T, D, 4, D_acc>::run(DotFunctor<T>(), std::plus<T>(), std::greater<T>(), data, p.data, upper_bound, acc);
}

} // namespace kche_tree
