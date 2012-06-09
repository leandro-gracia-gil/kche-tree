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
 * \file labeled_dataset.h
 * \brief Template for data sets containing an array of labeled feature vectors.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_LABELED_DATASET_H_
#define _KCHE_TREE_LABELED_DATASET_H_

#include "dataset.h"
#include "shared_ptr.h"

namespace kche_tree {

/**
 * \brief Object containing a reference-counted set of feature vectors with associated labels.
 *
 * Encapsulates a set of D-dimensional feature vectors and their labels that are shared
 * between different sets.
 *
 * \tparam ElementType Data type of the elements in each vector.
 * \tparam NumDimensions Number of dimensions of each vector.
 * \tparam LabelType Type of the labels associated to each of the feature vectors.
 */
template <typename ElementType, unsigned int NumDimensions, typename LabelType>
class LabeledDataSet : public Serializable<LabeledDataSet<ElementType, NumDimensions, LabelType> >,
                       public DataSet<ElementType, NumDimensions> {
public:
  /// Type of the elements in the data set.
  typedef ElementType Element;

  /// Number of dimensions of the vectors in the data set.
  static const unsigned int Dimensions = NumDimensions;

  /// Type of the labels of the feature vectors.
  typedef LabelType Label;

  /// Type of the associated non-labeled data set.
  typedef typename kche_tree::DataSet<Element, Dimensions> DataSet;

  /// Type of the feature vectors contained by the data set.
  typedef typename DataSet::Vector Vector;

  // Constructors and destructors.
  LabeledDataSet();
  LabeledDataSet(unsigned int size);
  LabeledDataSet(const Vector *vectors, const Label *labels, unsigned int size);
  LabeledDataSet(SharedArray<Vector> vectors, SharedArray<Label> labels, unsigned int size);
  LabeledDataSet(const LabeledDataSet &dataset, unsigned int *permutation);
  virtual ~LabeledDataSet();

  // Overriden initialization methods.
  virtual void reset_to_size(unsigned int size);

  // Comparison operators.
  bool operator == (const LabeledDataSet &dataset) const;
  bool operator != (const LabeledDataSet &dataset) const;

  // Comparison ignoring labels (content only).
  bool operator == (const DataSet &dataset) const;
  bool operator != (const DataSet &dataset) const;

  // Label-related methods.
  const Label& label(unsigned int index) const;
  Label& label(unsigned int index);

  // TODO: Add methods (iterators?) to manage entries by their labels.

private:
  // Implementation of the serializable concept.
  LabeledDataSet(std::istream &in, Endianness::Type endianness);
  void serialize(std::ostream &out) const;
  void swap(LabeledDataSet &dataset);

  using Serializable<LabeledDataSet>::check_serialized_type;
  using Serializable<LabeledDataSet>::serialize_type;
  using Serializable<LabeledDataSet>::type_name;

  friend std::istream& operator >> <>(std::istream &in, Serializable<LabeledDataSet> &dataset);
  friend std::ostream& operator << <>(std::ostream &out, const Serializable<LabeledDataSet> &dataset);

  SharedArray<Label> labels_; ///< Array of the vectors in the data set.
};

} // namespace kche_tree

// Template implementation.
#include "labeled_dataset.tpp"

#endif
