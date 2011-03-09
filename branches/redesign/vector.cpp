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
 * \file 	feature_vector.cpp
 * \brief 	Template class implementation for generic D-dimensional
 *              feature vectors.
 * \author	Leandro Graciá Gil, Puertos Ahuir Juan V.
 * \email       juanvi.puertos@gmail.com
*/



#include <cstdlib>
#include <new>
#include "feature_vector.h"

/**
 * Generic memory allocator operator for feature vector arrays.
 * Defined so that memory-aligned specializations can be defined if required.
 *
 * \param size  Total size in bytes of the objects to allocate.
 * \return Address to the new allocated memory.
 */
template <typename T, const unsigned int D>

void* kdt::Vector<T, D>::operator new [] (size_t size)
{ 
  void *p = malloc(size);
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
 * \param p     Pointer to the address to release.
 */
template <typename T, const unsigned int D>
void kdt::Vector<T, D>::operator delete [] (void *p)
{        
  free(p);
}

/**
 * Check if two feature vectors are exactly equal.
 *
 * \param p Feature vector being compared to.
 * \return \c true if equal, \c false otherwise.
 */
template <typename T, const unsigned int D>
bool kdt::Vector<T, D>::operator== (const Vector &p) const
{
  // Check that all components have the same value
  for (unsigned int d=0; d<D; ++d)
    if(data[d] != p.data[d])
      return false;

  return true;
}

/**
 * Check if two feature vectors are different equal.
 *
 * \param p Feature vector being compared to.
 * \return \c true if different, \c false otherwise.
 */
template <typename T, const unsigned int D>
bool kdt::Vector<T, D>::operator!= (const Vector &p) const
{
  for(unsigned int d=0; d<D; ++d)
    if(data[d] != p.data[d])
      return true;
 
  return false;
}


template <typename T, const unsigned int D>
const T & kdt::Vector<T, D>::operator [] (unsigned int i) const
{
  return data[i];
}

template <typename T, const unsigned int D>
T & kdt::Vector<T, D>::operator [] (unsigned int i)
{
  return data[i];
}


template <typename T, const unsigned int D>
T kdt::SquaredMetric<T, D>::distance(const Vector<T, D>&, const Vector<T, D>&) const
{

};
