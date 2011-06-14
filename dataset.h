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
 * \file dataset.h
 * \brief Template for data sets containing an array of feature vectors.
 * \author Leandro Graciá Gil
*/

#ifndef _DATASET_H_
#define _DATASET_H_

// STL streams, exceptions and runtime type information.
#include <iostream>
#include <stdexcept>
#include <typeinfo>

// Include raw type template and feature vectors.
#include "raw-types.h"
#include "vector.h"

namespace kche_tree {

// Forward-declaration of the class.
template <typename T, const unsigned int D>
class DataSet;

/// Input operator (read from stream). Throws an exception in case of error.
template <typename T, const unsigned int D>
std::istream & operator >> (std::istream &in, DataSet<T, D> &set);

/// Output operator (save to stream).
template <typename T, const unsigned int D>
std::ostream & operator << (std::ostream &out, const DataSet<T, D> &set);

/**
 * \brief Template for data sets containing feature vectors.
 *
 * Encapsulates a list of D-dimensional feature vectors with the provided size.
 * Depending on the constructor used, will allocate the vectors or will use an
 * existing array of them without taking any ownership.
 *
 * \tparam T Data type of the elements in each vector.
 * \tparam D Number of dimensions of each vector.
 */
template <typename T, const unsigned int D>
class DataSet {
public:
  // Constructors and destructors;
  DataSet();
  DataSet(unsigned int size);
  DataSet(Vector<T, D> *vectors, unsigned int size);
  ~DataSet();

  // Copy constructor and assignment operator.
  DataSet(const DataSet& dataset);
  DataSet& operator = (const DataSet& dataset);

  // Initialization methods.
  void reset_to_size(unsigned int size);

  // Attributes.
  unsigned int size() const { return size_; } ///< Returns the number of vectors in the data set.
  bool is_data_owner() const { return ptr_owner_; } ///< Tells if the vectors are owned by the object and hence will be released by it.

  // Subscript operators.
  const Vector<T, D>& operator [] (unsigned int index) const { return vectors_[index]; }
  Vector<T, D>& operator [] (unsigned int index) { return vectors_[index]; }

  // Comparison operators.
  bool operator == (const DataSet& dataset) const;
  bool operator != (const DataSet& dataset) const;

  // Stream operators.
  friend std::istream & operator >> <>(std::istream &in, DataSet &set);
  friend std::ostream & operator << <>(std::ostream &out, const DataSet &set);

private:
  Vector<T, D> *vectors_;
  unsigned int size_;
  bool ptr_owner_;
};

} // namespace kche_tree

// Template implementation.
#include "dataset.cpp"

#endif
