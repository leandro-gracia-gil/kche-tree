/***************************************************************************
 *   Copyright (C) 2011, 2012 by Leandro Graciá Gil                        *
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

#include <iostream>
#include <stdexcept>

// Include shared arrays, type traits and feature vectors.
#include "shared_ptr.h"
#include "serializable.h"
#include "traits.h"
#include "vector.h"

namespace kche_tree {

// Forward declarations.
template <typename ElementType, unsigned int NumDimensions, typename LabelType>
class KDTree;

/**
 * \brief Object containing a reference-counted set of feature vectors.
 *
 * Encapsulates a set of D-dimensional feature vectors that are shared
 * between different sets.
 *
 * \tparam ElementType Type of the elements in the feature vectors.
 * \tparam NumDimensions Number of dimensions of the feature vectors.
 */
template <typename ElementType, unsigned int NumDimensions>
class DataSet : public Serializable<DataSet<ElementType, NumDimensions> > {
public:
  /// Type of the elements in the data set.
  typedef ElementType Element;

  /// Number of dimensions of the vectors in the data set.
  static const unsigned int Dimensions = NumDimensions;

  /// Use the same vector type as the corresponding kd-tree.
  typedef typename KDTree<Element, Dimensions, void>::Vector Vector;

  // Constructors and destructors.
  DataSet();
  DataSet(unsigned int size);
  DataSet(const Vector *vectors, unsigned int size);
  DataSet(SharedArray<Vector> vectors, unsigned int size);
  DataSet(const DataSet &dataset, unsigned int *permutation);
  virtual ~DataSet();

  // Initialization methods.
  virtual void reset_to_size(unsigned int size);

  template <typename RandomGenerator>
  void set_random_values(RandomGenerator &generator);

  // Generic attributes.
  unsigned int size() const { return size_; } ///< Returns the number of vectors in the data set.
  const SharedArray<Vector> vectors() const { return vectors_; } ///< Return the shared pointer of all contiguous vectors.
  long use_count() const { return vectors_.use_count(); } ///< Return the number of references to the cointained vectors.

  // Index permutation methods.
  unsigned int get_permuted_index(unsigned int index) const;
  unsigned int get_original_index(unsigned int index) const;

  // Permutation-sensitive accessors.
  const Vector& get_permuted(unsigned int permuted_index) const;
  Vector& get_permuted(unsigned int permuted_index);

  // Subscript operators.
  const Vector& operator [] (unsigned int index) const;
  Vector& operator [] (unsigned int index);

  // Comparison operators.
  bool operator == (const DataSet &dataset) const;
  bool operator != (const DataSet &dataset) const;

  /// Const iterator for the columns of the data set. Iterates through the i-dimensional element of each vector.
  class ColumnConstIterator : public std::iterator<std::bidirectional_iterator_tag, Element> {
  public:
    ColumnConstIterator(const DataSet &dataset, unsigned int column, unsigned int row);
    ColumnConstIterator(const ColumnConstIterator &iterator);

    bool operator == (const ColumnConstIterator &iterator) const;
    bool operator != (const ColumnConstIterator &iterator) const;

    ColumnConstIterator& operator ++ ();
    ColumnConstIterator operator ++ (int);

    ColumnConstIterator& operator -- ();
    ColumnConstIterator operator -- (int);

    const Element& operator * ();
    const Element* operator -> ();

  private:
    const DataSet &dataset_;
    unsigned int column_;
    unsigned int row_;
  };

  // Iterators to access columns of the data set.
  ColumnConstIterator column_begin(unsigned int column) const;
  ColumnConstIterator column_end(unsigned int column) const;

protected:
  // Implementation of the serializable concept.
  DataSet(std::istream &in, Endianness::Type endianness);
  void serialize(std::ostream &out) const;
  void swap(DataSet &dataset);

  friend std::istream& operator >> <>(std::istream &in, Serializable<DataSet> &dataset);
  friend std::ostream& operator << <>(std::ostream &out, const Serializable<DataSet> &dataset);

  SharedArray<Vector> vectors_; ///< Array of the vectors in the data set. Optionally aligned to the 16-byte boundary for SSE optimizations.
  ScopedArray<unsigned int> permuted_to_original_; ///< Index array to transform from permuted indices to original ones.
  ScopedArray<unsigned int> original_to_permuted_; ///< Index array to transform from original indices to permuted ones.
  unsigned int size_; ///< Number of vectors in the data set.
  static const uint16_t version[2]; ///< Tuple of major and minor version of the current data set serialization format.
};

} // namespace kche_tree

// Template implementation.
#include "dataset.tpp"

#endif
