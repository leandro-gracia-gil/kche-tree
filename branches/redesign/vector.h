/***************************************************************************
 *   Copyright (C) 2010-2011 by                                            *
 *                                                                         *
 *   Leandro Graciá Gil                                                    *
 *   leandro.gracia.gil@gmail.com                                          *
 *                                                                         *
 *   Juan V. Puertos Ahuir                                                 *
 *   juanvi.puertos@gmail.com                                              *
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
 * \file 	feature_vector.h
 * \brief 	Template class for generic D-dimensional feature vectors.
 * \author	Leandro Graciá Gil, Puertos Ahuir Juan V.
 * \email       juanvi.puertos@gmail.com
*/

#ifndef _FEATURE_VECTOR_H_
#define _FEATURE_VECTOR_H_

#include <functional>
#include <new>

//namespace kdt
//{

template <typename T, const unsigned int D>
class Metric;

template <typename T, const unsigned int D>
class Vector
{
 public:
  Vector() {};
  bool operator == (const Vector&) const;   ///< Equality comparison operator.
  bool operator != (const Vector&) const;   ///< Non-equality comparison operator.
  void* operator new[] (size_t size); 	///< Standard allocation for arrays of feature vectors.
  void  operator delete[] (void*); 	///< Standard deallocation for arrays of feature vectors.
  const T & operator[] (unsigned int) const; ///< Const subscript operator.
  T & operator[] (unsigned int); 		///< Subscript operator.
  T distance(const Vector&, const Vector&) const; ///< Distance operator. Strategy pattern.
  
 private:
  T data[D];
  Metric<T, D> metric;
};

template <typename T, const unsigned int D>
class VDistance : public std::binary_function<VDistance<T,D>, VDistance<T,D>, bool>
{
 public:
  VDistance();
  VDistance(unsigned int, T distance);
  bool operator() (const VDistance<T,D>&, const VDistance<T,D>&) const;
 private:
  unsigned int index; ///< Index of the feature vector in the data set.
  T distance;         ///< Distance of the referenced element to an implicit point.
};

template <typename T, const unsigned int D>
class Metric
{
 public:
  T distance(const Vector<T, D>&, const Vector<T, D>&) const;
};

template <typename T, const unsigned int D>
class SquaredMetric : public Metric<T,D>
{
 public:
  T distance(const Vector<T, D>&, const Vector<T, D>&) const;
};

//}
#endif
