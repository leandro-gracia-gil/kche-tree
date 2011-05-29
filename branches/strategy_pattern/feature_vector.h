/***************************************************************************
 *   Copyright (C) 2010 by Leandro Graciá Gil                              *
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
 * \file feature_vector.h
 * \brief Template for generic D-dimensional feature vectors.
 * \author Leandro Graciá Gil, Juan V. Puertos
*/

#ifndef _FEATURE_VECTOR_H_
#define _FEATURE_VECTOR_H_

// Include STL binary predicates.
#include <functional>
#include <new>

template <typename T, const unsigned int D>
  struct feature_vector;

template <typename T, const unsigned int D>
class IMetric
{
 public:
  IMetric() {};
  virtual T distance_to(const feature_vector<T, D> &,
			const feature_vector<T, D> &) = 0;
  
  virtual T distance_to(const feature_vector<T, D> &,
			const feature_vector<T, D> &, T) const = 0;

};

template <typename T, const unsigned int D>
  class EuclideanMetric : public IMetric<T, D>
{
 public:
  EuclideanMetric() { };
  inline T distance_to(const feature_vector<T, D> &a,
		       const feature_vector<T, D> &b)
  {
    T acc = (T) 0;
    for (unsigned int i=0; i<D; ++i)
      acc += (a[i] - b[i]) * (a[i] - b[i]);
    return acc;
  };

  inline T distance_to(const feature_vector<T, D> &a,
		       const feature_vector<T, D> &b,
		       T upper_bound) const
  {
    const unsigned int D_acc = (unsigned int) (0.25f * D);

    // Squared distance in two steps: first accumulate without comparisons.
    T acc = (T) 0;
    for (unsigned int i=0; i<D_acc; ++i)
      acc += (a[i] - b[i]) * (a[i] - b[i]);

    // Second step: accumulate comparing with upper bound.
    for (unsigned int i=D_acc; i<D; ++i) {
      acc += (a[i] - b[i]) * (a[i] - b[i]);
      if (acc > upper_bound)
	break;
    }

    return acc;
  }

};

// Chebyshev distance
// Manhatan distance

/**
 * \brief Template for D-dimensional feature vectors.
 *
 * Encapsulates a D-dimensional array with an extra entry of the same type for the class label.
 * Has been compared to the direct use of arrays with no change in efficiency.
 *
 * \tparam T Data type of the elements in the vector.
 * \tparam D Number of dimensions of the vector.
 */
template <typename T, const unsigned int D>
struct feature_vector {
public:

  /// Data array.
  T data[D];
  IMetric<T, D>* metric;

  // Constructors.
  feature_vector() { metric = new EuclideanMetric<T,D>(); } ///< Default constructor.
  //feature_vector(IMetric<T,D>& m) { metric = m;} ///< Default constructor.
  feature_vector(IMetric<T,D>& m, T value) {
    metric = m;
    for (unsigned int d=0; d<D; ++d)
      data[d] = value;
  } ///< Value initialization constructor.

  // Subscript operators.
  const T & operator [] (unsigned int index) const { return data[index]; } ///< Const subscript operator.
  T & operator [] (unsigned int index) { return data[index]; } ///< Subscript operator.

  // Memory operators: used to allow memory-aligned specializations. For example, for SSE optimizations.
  void *operator new [] (size_t size); ///< Standard allocation for arrays of feature vectors.
  void  operator delete [] (void *p); ///< Standard deallocation for arrays of feature vectors.

  // Comparison operators.
  bool operator == (const feature_vector &p) const; ///< Equality comparison operator.
  bool operator != (const feature_vector &p) const; ///< Non-equality comparison operator.

  // Squared distance operators for two D-dimensional points of type T.
  
  inline T distance_to(const feature_vector &p) const;

  inline T distance_to(const feature_vector &p, T upper_bound) const; ///< Squared distance to a point with an upper bound.
  
  void set_metric();
 
} __attribute__((packed));

/**
 * \brief Vector-distance structure. References a feature vector by its index and its squared distance to another implicit vector.
 *
 * Implements its own comparison function with the parenthesis operator for STL-based algorithm use.
 *
 * \tparam T  Type used to encode the distance between two feature vectors. Should be the same than the data from the vectors.
 */
template <typename T>
struct vector_distance : public std::binary_function <vector_distance<T>, vector_distance<T>, bool> {

  unsigned int index; ///< Index of the feature vector in the data set.
  T squared_distance; ///< Squared distance of the referenced element to an implicit point.

  // Default and convenience constructors.
  vector_distance() {}
  vector_distance(unsigned int index, T squared_distance) : index(index), squared_distance(squared_distance) {}

  /// Distance comparison operator for vector_distances. Allows vector_distance objects to be used as STL comparison functors.
  bool operator () (const vector_distance &v1, const vector_distance &v2) const {
    return v1.squared_distance < v2.squared_distance;
  }
};

// Template implementation.
#include "feature_vector.cpp"

#endif
