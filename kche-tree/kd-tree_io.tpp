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
 * \file kd-tree_io.tpp
 * \brief Template implementations for stream operations in kd-trees.
 * \author Leandro Graciá Gil
 */

// STL smart pointers, exceptions, strings and runtime type information.
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>

namespace kche_tree {

// KD-Tree serialization settings.
template <typename T, unsigned int D, typename L> const uint16_t KDTree<T, D, L>::version[2] = { 2, 0 };
template <typename T, unsigned int D, typename L> const uint16_t KDTree<T, D, L>::signature = 0xCAFE;

// KD-Tree content verification
template <bool enabled> struct VerifyKDTreeContents;

/// Verify the loaded kd-tree structural properties. Specialization to enable the operation.
template <>
struct VerifyKDTreeContents<true> {
  template <typename T, unsigned int D>
  static void verify(const KDNode<T, D> *root, const DataSet<T, D> &data) {
    KCHE_TREE_DCHECK(root);
    root->verify_properties(data, 0);
  }
};

/// Verify the loaded kd-tree structural properties. Specialization to disable the operation.
template <>
struct VerifyKDTreeContents<false> {
  template <typename T, unsigned int D>
  static void verify(const KDNode<T, D> *root, const DataSet<T, D> &data) {}
};

/**
 * \brief Serialize a kd-tree object into an output stream.
 *
 * The permuted train set stored in the kd-tree is also serialized, including any possible label information.
 *
 * \param out The output stream where the kd-tree should be serialized.
 * \exception std::runtime_error Thrown in case of error.
 */
template <typename T, unsigned int D, typename L>
void KDTree<T, D, L>::serialize(std::ostream &out) const {

  // Write file version.
  kche_tree::serialize(KDTree::version, out);
  if (!out.good())
    throw std::runtime_error("error writing kd-tree format version");

  KCHE_TREE_DCHECK(data_);
  if (!data_->size())
    return;

  // Write the tree data set. Will throw std::runtime_error on failure.
  out << *data_;

  if (!root_)
    throw std::runtime_error("invalid kd-tree structure: data but no nodes");

  // Write the kd-tree structure recursively. Will throw std::runtime_error on failure.
  root_->serialize(out);

  // Write a 2-byte signature at the end.
  kche_tree::serialize(signature, out);
  if (!out.good())
    throw std::runtime_error("error writing the file signature");
}

/**
 * \brief Deserializes a kd-tree object from an output stream.
 *
 * The permuted train set stored in the kd-tree is also deserialized, including any possible label information.
 *
 * \param in The input stream where the kd-tree should be deserialized from.
 * \param endianness Endianness of the input data.
 * \exception std::runtime_error Thrown in case of error.
 */
template <typename T, unsigned int D, typename L>
KDTree<T, D, L>::KDTree(std::istream &in, Endianness::Type endianness) {

  // Read format version.
  uint16_t version[2];
  deserialize(version, in, endianness);
  if (!in.good())
    throw std::runtime_error("error reading version data");

  // Check supported file versions.
  if (version[0] != KDTree::version[0] || version[1] != KDTree::version[1]) {
    std::string error_msg = "unsupported kd-tree version: required ";
    error_msg += KDTree::version[0];
    error_msg += ".";
    error_msg += KDTree::version[1];
    error_msg += ", found ";
    error_msg += version[0];
    error_msg += ".";
    error_msg += version[1];
    throw std::runtime_error(error_msg);
  }

  // Read the kd-tree data set.
  data_.reset(new DataSet());
  in >> *data_;

  if (!data_->size())
    return;

  // Read the tree structure from the stream.
  root_.reset(new KDNode(in, endianness));

  // Read and check the signature value.
  uint16_t signature;
  deserialize(signature, in, endianness);
  if (in.fail() || signature != KDTree::signature)
    throw std::runtime_error("error reading kd-tree signature, data might be corrupted or incomplete");

  // Verify kd-tree contents if enabled by the settings. Will throw std::runtime_error if not valid.
  VerifyKDTreeContents<Settings::verify_kdtree_after_deserializing>::verify(root_.get(), *data_);
}

/**
 * \brief Swaps the contents of two kd-trees.
 *
 * Used to prevent corrupting objects in case of partial deserializations.
 *
 * \param kdtree The kd-tree object whose contents should swap with.
 */
template <typename T, unsigned int D, typename L>
void KDTree<T, D, L>::swap(KDTree &kdtree) {
  kdtree.data_.swap(data_);
  kdtree.root_.swap(root_);
}

/**
 * \brief Build a kd-tree branch node from the data in a binary input stream.
 *
 * \param in Input stream.
 * \param endianness Endianness of the serialized data.
 * \exception std::runtime_error Thrown in case of error reading or processing the node data.
 */
template <typename T, unsigned int D>
KDNode<T, D>::KDNode(std::istream &in, Endianness::Type endianness)
    : left_branch(NULL),
      right_branch(NULL) {

  // Read node data.
  deserialize(split_element, in, endianness);
  deserialize(is_leaf, in, endianness);
  if (!in.good())
    throw std::runtime_error("error reading node data");

  // Process the left branch or leaf.
  if (is_leaf & left_bit)
    left_leaf = new KDLeaf(in, endianness);
  else
    left_branch = new KDNode(in, endianness);

  // Process the right branch or leaf.
  if (is_leaf & right_bit)
    right_leaf = new KDLeaf(in, endianness);
  else
    right_branch = new KDNode(in, endianness);
}

/**
 * \brief Build a kd-tree branch node from the data in a binary input stream.
 *
 * \param in Input stream.
 * \param endianness Endianness of the serialized data.
 * \exception std::runtime_error Thrown in case of error reading or processing the leaf node data.
 */
template <typename T, unsigned int D>
KDLeaf<T, D>::KDLeaf(std::istream &in, Endianness::Type endianness) {

  // Read leaf node data.
  deserialize(first_index, in, endianness);
  deserialize(num_elements, in, endianness);
  if (!in.good())
    throw std::runtime_error("error reading leaf node data");
}

/**
 * \brief Write a kd-tree branch node to a binary ouput stream.
 *
 * \param out Output stream.
 * \exception std::runtime_error Thrown in case of error writing the data.
 */
template <typename T, unsigned int D>
void KDNode<T, D>::serialize(std::ostream &out) {

  // Write split value and node axis/leaf information.
  kche_tree::serialize(split_element, out);
  kche_tree::serialize(is_leaf, out);
  if (!out.good())
    throw std::runtime_error("error writing internal node data");

  // Process the left branch or leaf.
  if (is_leaf & left_bit)
    left_leaf->serialize(out);
  else
    left_branch->serialize(out);

  // Process the right branch or leaf.
  if (is_leaf & right_bit)
    right_leaf->serialize(out);
  else
    right_branch->serialize(out);
}

/**
 * \brief Write a kd-tree leaf node to a binary ouput stream.
 *
 * \param out Output stream.
 * \exception std::runtime_error Thrown in case of error writing the data.
 */
template <typename T, unsigned int D>
void KDLeaf<T, D>::serialize(std::ostream &out) {

  // Write left leaf node information.
  kche_tree::serialize(first_index, out);
  kche_tree::serialize(num_elements, out);
  if (!out.good())
    throw std::runtime_error("error writing leaf node data");
}

} // namespace kche_tree
