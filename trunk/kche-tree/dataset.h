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

#ifndef _KCHE_TREE_DATASET_H_
#define _KCHE_TREE_DATASET_H_

// STL streams, exceptions and runtime type information.
#include <iostream>
#include <stdexcept>
#include <typeinfo>

// Include smart pointers, type traits and feature vectors.
#include "smart_ptr.h"
#include "traits.h"
#include "vector.h"

namespace kche_tree {

// Forward-declaration of the class.
template <typename T, const unsigned int D>
class DataSet;

/**
 * \brief Load the contents of the input stream from the data set.
 * The original contents of the \a dataset object are not modified in case of error.
 *
 * \param in Input stream.
 * \param dataset DataSet to be deserialized.
 * \exception std::runtime_error Thrown in case of reading or validation error.
 */
template <typename T, const unsigned int D>
std::istream & operator >> (std::istream &in, DataSet<T, D> &dataset);

/**
 * \brief Save the contents of the data set to the output stream.
 *
 * \note Data is serialized with the same endianness as the local host.
 * \param out Output stream.
 * \param dataset DataSet to be serialized.
 * \exception std::runtime_error Thrown in case of writing error.
 */
template <typename T, const unsigned int D>
std::ostream & operator << (std::ostream &out, const DataSet<T, D> &dataset);

/**
 * \brief Object containing a reference-counted set of feature vectors.
 *
 * Encapsulates a set of D-dimensional feature vectors that are shared
 * between different sets.
 *
 * \tparam T Data type of the elements in each vector.
 * \tparam D Number of dimensions of each vector.
 */
template <typename T, const unsigned int D>
class DataSet {
public:
  /// Use the global vector type by default.
  typedef typename Settings<T, D>::VectorType VectorType;

  // Constructors and destructors;
  DataSet(); ///< Create an empty data set.
  DataSet(unsigned int size); ///< Create a data set of the specified size.
  DataSet(SharedArray<VectorType> vectors, unsigned int size); ///< Create a data set object over shared vector array of the specified size.

  // Initialization methods.
  void reset_to_size(unsigned int size); ///< Reset the data set to an uninitialized version of the specified size.

  // Attributes.
  unsigned int size() const { return size_; } ///< Returns the number of vectors in the data set.
  const SharedArray<VectorType> vectors() const { return vectors_; } ///< Return the shared pointer of all contiguous vectors.
  long use_count() const { return vectors_.use_count(); } ///< Return the number of references to the cointained vectors.

  // Subscript operators.
  const VectorType &operator [] (unsigned int index) const { return vectors_[index]; } ///< Access vectors of the data set without modifying them.
  VectorType &operator [] (unsigned int index); ///< Access vectors of the data set. Will make a separate copy of the contents if shared with something else.

  // Comparison operators.
  bool operator == (const DataSet& dataset) const; ///< Check if the data set is equal to some other. May be optimized if \link kche_tree::has_trivial_equal has_trivial_equal::value\endlink is \c true.
  bool operator != (const DataSet& dataset) const; ///< Check if the data set is different to some other. May be optimized if \link kche_tree::has_trivial_equal has_trivial_equal::value\endlink is \c true.

  // Stream operators.
  friend std::istream & operator >> <>(std::istream &in, DataSet &dataset);
  friend std::ostream & operator << <>(std::ostream &out, const DataSet &dataset);

private:
  SharedArray<VectorType> vectors_; ///< Array of the vectors in the data set.
  uint32_t size_; ///< Number of vectors in the data set.
  static const uint16_t version[2]; ///< Tuple of major and minor version of the current data set serialization format.
};

} // namespace kche_tree

// Template implementation.
#include "dataset.tpp"

#endif
