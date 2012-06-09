/***************************************************************************
 *   Copyright (C) 2010, 2011, 2012 by Leandro Graciá Gil                  *
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
 * \file kd-tree.h
 * \brief Template for generic kd-trees.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_KD_TREE_H_
#define _KCHE_TREE_KD_TREE_H_

// Include STL streams and vectors for K neighbours output.
#include <iostream>
#include <vector>

// Include k-neighbours containers (k-heaps and k-vectors).
#include "k-heap.h"
#include "k-vector.h"

// Other includes from the library.
#include "dataset.h"
#include "kd-node.h"
#include "labeled_dataset.h"
#include "metrics.h"
#include "neighbor.h"
#include "serializable.h"
#include "traits.h"
#include "utils.h"
#include "vector.h"

namespace kche_tree {

// Forward-declare the class.
template <typename ElementType, unsigned int NumDimensions, typename LabelType = void>
class KDTree;

/**
 * \brief Standard input stream operator. Loads the kd-tree from a stream in binary format.
 *
 * The original contents of the \a kdtree object are not modified in case of error.
 *
 * \param in Input stream.
 * \param kdtree Kd-tree where results will be saved.
 * \return Input stream after reading the data.
 * \exception std::runtime_error Thrown in case of error reading or processing the kd-tree.
 */
//template <typename T, unsigned int D, typename L>
//std::istream & operator >> (std::istream &in, KDTree<T, D, L> &kdtree);

/**
 * \brief Standard output stream operator.
 *
 * Saves the kd-tree into a stream in binary format.
 * \note Data is serialized with the same endianness as the local host.
 *
 * \param out Output stream.
 * \param kdtree Kd-tree being saved.
 * \return Output stream after writting the data.
 * \exception std::runtime_error Thrown in case of error writing the kd-tree.
 */
//template <typename T, unsigned int D, typename L>
//std::ostream & operator << (std::ostream &out, const KDTree<T, D, L> &kdtree);

/**
 * \brief Template for generic cache-aware kd-trees of any type.
 *
 * Element insertions and deletions are not currently supported in behalf of a design based on cache efficiency.
 * \warning This class is currently not thread-safe. This feature will be added in future releases.
 *
 * \tparam ElementType Type of the elements in the kd-tree. Requires copy construction, assignment and the < > == operators. Further operators may be required depending on the use. See the \c custom_type example for details.
 * \tparam NumDimensions Number of dimensions of the feature vectors.
 * \tparam LabelType Type of the labels associated to the feature vectors in the data set. Defaults to void, which means unlabeled data sets.
 */
template <typename ElementType, unsigned int NumDimensions, typename LabelType>
class KDTree : public Serializable<KDTree<ElementType, NumDimensions, LabelType> > {
public:
  /// Type of the elements in the kd-tree.
  typedef ElementType Element;

  /// Number of dimensions in the kd-tree.
  static unsigned const int Dimensions = NumDimensions;

  /// Type of the labels associated to the feature vectors. Equals to void if no labels are used.
  typedef LabelType Label;

  /// Type of the feature vectors used by this kd-tree.
  typedef kche_tree::Vector<Element, Dimensions> Vector;

  /// Type of the data sets for use with this kd-tree. Uses the DataSet template if Label is void and LabeledDataSet otherwise.
  typedef typename TypeBranch<IsSame<Label, void>::value,
                              kche_tree::DataSet<Element, Dimensions>,
                              kche_tree::LabeledDataSet<Element, Dimensions, Label> >::Result DataSet;

  /// Type encoding the distances between elements in the kd-tree.
  typedef typename Traits<Element>::Distance Distance;

  /// Type of a kd-tree neighbour result.
  typedef kche_tree::Neighbor<Distance> Neighbor;

  /// Type of the k-neighbour results.
  typedef std::vector<Neighbor> KNeighbors;

  /// Type of the default metric used for search methods.
  typedef EuclideanMetric<Element, Dimensions> DefaultMetric;

  /// Type of the smart const references to a distance. Uses values or const references depending on what is smaller and appropriate. Should be transparent to the user.
  typedef typename RParam<Distance>::Type ConstRef_Distance;

  /// Default size for kd-tree leaf node buckets.
  static const unsigned int DefaultBucketSize = 32;

  /// Default constructor. Creates an empty and uninitialized kd-tree.
  KDTree();

  /// Builds a kd-tree using a data set as training data.
  KDTree(const DataSet &train_set, unsigned int bucket_size = DefaultBucketSize);

  // Basic kd-tree operations.
  bool build(const DataSet &train_set, unsigned int bucket_size = DefaultBucketSize); ///< Build a kd-tree from a set of training vectors. Cost: O(n log² n).

  #ifdef KCHE_TREE_DISABLE_CPP1X
  template <template <typename, typename> class KContainer, typename M>
  void knn(const Vector &p, unsigned int K, KNeighbors &output, const M &metric = DefaultMetric(), ConstRef_Distance epsilon = Traits<Distance>::zero(), bool ignore_p_in_tree = false) const; ///< Get the K nearest neighbours of a point. Estimated average cost: O(log K log n).
  #else
  template <template <typename, typename> class KContainer = KVector, typename M = DefaultMetric>
  void knn(const Vector &p, unsigned int K, KNeighbors &output, const M &metric = M(), ConstRef_Distance epsilon = Traits<Distance>::zero(), bool ignore_p_in_tree = false) const; ///< Get the K nearest neighbours of a point. Estimated average cost: O(log K log n).
  #endif

  #ifdef KCHE_TREE_DISABLE_CPP1X
  template <typename M>
  void all_in_range(const Vector &p, ConstRef_Distance distance, std::vector<Neighbor> &output, const M &metric = DefaultMetric(), bool ignore_p_in_tree = false) const; ///< Get all neighbours within a distance from a point. Estimated average Cost: O(log m log n) depending on the number of results m.
  #else
  template <typename M = DefaultMetric>
  void all_in_range(const Vector &p, ConstRef_Distance distance, std::vector<Neighbor> &output, const M &metric = M(), bool ignore_p_in_tree = false) const; ///< Get all neighbours within a distance from a point. Estimated average Cost: O(log m log n) depending on the number of results m.
  #endif

  // Access to the data stored within the kd-tree.
  const DataSet& data() const;

  // Stream operators.
  //friend std::istream& operator >> <>(std::istream &in, KDTree &kdtree);
  //friend std::ostream& operator << <>(std::ostream &out, const KDTree &kdtree);

  // Kd-tree properties.
  unsigned int size() const; ///< Get the number of elements stored in the tree.

private:
  // Implementation of the serializable concept.
  KDTree(std::istream &in, Endianness::Type endianness);
  void serialize(std::ostream &out) const;
  void swap(KDTree &kdtree);

  friend std::istream& operator >> <>(std::istream &in, Serializable<KDTree> &kdtree);
  friend std::ostream& operator << <>(std::ostream &out, const Serializable<KDTree> &kdtree);

  /// Type of the internal nodes using in the tree.
  typedef kche_tree::KDNode<Element, Dimensions> KDNode;

  /// Type of the leaf nodes using in the tree.
  typedef kche_tree::KDLeaf<Element, Dimensions> KDLeaf;

  // Kd-tree data.
  ScopedPtr<KDNode> root_; ///< Root node of the tree. Will point to \c NULL in empty trees.
  ScopedPtr<DataSet> data_; ///< Data of the kd-tree. Consists of a permuted version of the training set created while building the tree.

  // Serialization settings.
  static const uint16_t version[2]; ///< Tuple of major and minor version of the current kd-tree serialization format.
  static const uint16_t signature; ///< Signature value used to check the end of data according to the current format.
};

} // namespace kche_tree

// Template implementation.
#include "kd-tree.tpp"
#include "kd-tree_io.tpp"

#endif
