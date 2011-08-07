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
template <typename T, const unsigned int D> const uint16_t KDTree<T, D>::version[2] = { 2, 0 };
template <typename T, const unsigned int D> const uint16_t KDTree<T, D>::signature = 0xCAFE;

// KD-Tree content verification
template <bool enabled> struct VerifyKDTreeContents;

/// Verify the loaded kd-tree structural properties. Specialization to enable the operation.
template <>
struct VerifyKDTreeContents<true> {
  template <typename T, const unsigned int D>
  static void verify(const KDNode<T, D> *root, const DataSet<T, D> &data) {
    assert(root);
    root->verify_properties(data, 0);
  }
};

/// Verify the loaded kd-tree structural properties. Specialization to disable the operation.
template <>
struct VerifyKDTreeContents<false> {
  template <typename T, const unsigned int D>
  static void verify(const KDNode<T, D> *root, const DataSet<T, D> &data) {}
};

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
template <typename T, const unsigned int D>
std::istream & operator >> (std::istream &in, KDTree<T, D> &kdtree) {

  // Type aliases.
  typedef KDTree<T, D> KDTreeType;
  typedef typename TypeSettings<T, D>::DataSetType DataSetType;

  // Check the state of the input stream.
  if (!in.good())
    throw std::runtime_error("input stream not ready");

  // Read format endianness.
  uint8_t endianness_raw;
  deserialize(endianness_raw, in);
  if (!in.good())
    throw std::runtime_error("error reading endianness type");

  // Validate endianness.
  Endianness::Type endianness = static_cast<Endianness::Type>(endianness_raw);
  if (endianness != Endianness::LittleEndian && endianness != Endianness::BigEndian)
    throw std::runtime_error("invalid endianness value");

  // Read file version.
  uint16_t version[2];
  deserialize(version, in, endianness);
  if (!in.good())
    throw std::runtime_error("error reading version data");

  // Check the serialized type. Will throw std::runtime_error in case it doesn't match.
  // This will implicitely check both the type of T and the number of dimensions.
  Traits<KDTreeType>::check_serialized_type(in, endianness);

  // Check supported file versions.
  if (version[0] != KDTreeType::version[0] || version[1] != KDTreeType::version[1]) {
    std::string error_msg = "unsupported kd-tree version: required ";
    error_msg += KDTree<T, D>::version[0];
    error_msg += ".";
    error_msg += KDTree<T, D>::version[1];
    error_msg += ", found ";
    error_msg += version[0];
    error_msg += ".";
    error_msg += version[1];
    throw std::runtime_error(error_msg);
  }

  // Read the kd-tree data set. Will also check the type and dimensions of the tree.
  DataSetType dataset;
  in >> dataset;

  // Allocate and read the permutation array.
  uint32_t num_elements = dataset.size();
  ScopedArray<uint32_t> permutation(new uint32_t[num_elements]);
  deserialize_array(permutation.get(), num_elements, in, endianness);
  if (!in.good())
    throw std::runtime_error("error reading kd-tree data permutation");

  // Build and validate the inverse permutation array.
  ScopedArray<uint32_t> inverse_permutation(new uint32_t[num_elements]);
  memset(inverse_permutation.get(), 0xFF, num_elements * sizeof(uint32_t));

  for (uint32_t i=0; i<num_elements; ++i) {
    if (permutation[i] >= num_elements || inverse_permutation[permutation[i]] != 0xFFFFFFFF)
      throw std::runtime_error("invalid kd-tree permutation data");
    inverse_permutation[permutation[i]] = i;
  }

  // Read the tree structure from the stream.
  typedef KDNode<T, D> NodeType;
  ScopedPtr<NodeType> root(new NodeType(in, endianness));

  // Read the signature value.
  uint16_t signature;
  deserialize(signature, in, endianness);
  if (in.fail() || signature != kdtree.signature)
    throw std::runtime_error("error reading kd-tree signature");

  // Verify kd-tree contents if enabled by the settings. Will throw std::runtime_error if not valid.
  VerifyKDTreeContents<Settings<T>::verify_kdtree_after_deserializing>::verify(root.get(), dataset);

  // Set the kd-tree contents. The dataset vectors are reference-counted.
  kdtree.data = dataset;
  swap(kdtree.root, root);
  swap(kdtree.permutation, permutation);
  swap(kdtree.inverse_permutation, inverse_permutation);

  // Return the input stream.
  return in;
}

/**
 * Standard output stream operator.
 * Saves the kd-tree into a stream in binary format.
 *
 * \param out Output stream.
 * \param kdtree Kd-tree being saved.
 * \return Output stream after writting the data.
 * \exception std::runtime_error Thrown in case of error writing the kd-tree.
 */
template <typename T, const unsigned int D>
std::ostream & operator << (std::ostream &out, const KDTree<T, D> &kdtree) {

  // Check the state of the output stream.
  if (!out.good())
    throw std::runtime_error("output stream not ready");

  // Serialize the endianness being used.
  uint8_t endianness_raw = Endianness::endianness();
  serialize(endianness_raw, out);
  if (!out.good())
    throw std::runtime_error("error writing endianness information");

  // Write file version.
  typedef KDTree<T, D> KDTreeType;
  serialize(KDTreeType::version, out);
  if (!out.good())
    throw std::runtime_error("error writing kd-tree format version");

  // Serialize the KDTree type into the stream. This implicitely serializes both the type T and the number of dimensions.
  Traits<KDTreeType>::serialize_type(out);
  if (!out.good())
    throw std::runtime_error("error serializing the type information");

  // Write the tree data set. Will throw std::runtime_error on failure.
  out << kdtree.data;

  // Write the tree data permutation.
  serialize_array(kdtree.permutation.get(), kdtree.size(), out);
  if (!out.good())
    throw std::runtime_error("error writing the kd-tree permutation data");

  // Write the kd-tree structure recursively. Will throw std::runtime_error on failure.
  kdtree.root->write_to_binary_stream(out);

  // Write a 2-byte signature at the end: simplifies error checking.
  serialize(kdtree.signature, out);
  if (!out.good())
    throw std::runtime_error("error writing the file signature");

  // Return the output stream.
  return out;
}

/**
 * Build a kd-tree branch node from the data in a binary input stream.
 * \warning A std::runtime_error exception may be thrown in case of error reading the data.
 *
 * \param in Input stream.
 * \param endianness Endianness of the serialized data. Defaults to host's endianness.
 * \exception std::runtime_error Thrown in case of error reading or processing the node data.
 */
template <typename T, const unsigned int D>
KDNode<T, D>::KDNode(std::istream &in, Endianness::Type endianness)
  : left_branch(NULL),
    right_branch(NULL) {

  // Read node data.
  deserialize(split_value, in, endianness);
  deserialize(is_leaf, in, endianness);
  if (!in.good())
    throw std::runtime_error("error reading node data");

  // Type aliases.
  typedef KDNode<T, D> NodeType;
  typedef KDLeaf<T, D> LeafType;

  // Process the left branch or leaf.
  if (is_leaf & left_bit)
    left_leaf = new LeafType(in);
  else
    left_branch = new NodeType(in);

  // Process the right branch or leaf.
  if (is_leaf & right_bit)
    right_leaf = new LeafType(in);
  else
    right_branch = new NodeType(in);
}

/**
 * Build a kd-tree branch node from the data in a binary input stream.
 * \warning A std::runtime_error exception may be thrown in case of error reading the data.
 *
 * \param in Input stream.
 * \param endianness Endianness of the serialized data. Defaults to host's endianness.
 * \exception std::runtime_error Thrown in case of error reading or processing the leaf node data.
 */
template <typename T, const unsigned int D>
KDLeaf<T, D>::KDLeaf(std::istream &in, Endianness::Type endianness) {

  // Read leaf node data.
  deserialize(first_index, in, endianness);
  deserialize(num_elements, in, endianness);
  if (!in.good())
    throw std::runtime_error("error reading leaf node data");
}

/**
 * Write a kd-tree branch node to a binary ouput stream.
 *
 * \param out Output stream.
 * \exception std::runtime_error Thrown in case of error writing the data.
 */
template <typename T, const unsigned int D>
void KDNode<T, D>::write_to_binary_stream(std::ostream &out) {

  // Write split value and node axis/leaf information.
  serialize(split_value, out);
  serialize(is_leaf, out);
  if (!out.good())
    throw std::runtime_error("error writing internal node data");

  // Process the left branch or leaf.
  if (is_leaf & left_bit)
    left_leaf->write_to_binary_stream(out);
  else
    left_branch->write_to_binary_stream(out);

  // Process the right branch or leaf.
  if (is_leaf & right_bit)
    right_leaf->write_to_binary_stream(out);
  else
    right_branch->write_to_binary_stream(out);
}

/**
 * Write a kd-tree leaf node to a binary ouput stream.
 *
 * \param out Output stream.
 * \exception std::runtime_error Thrown in case of error writing the data.
 */
template <typename T, const unsigned int D>
void KDLeaf<T, D>::write_to_binary_stream(std::ostream &out) {

  // Write left leaf node information.
  serialize(first_index, out);
  serialize(num_elements, out);
  if (!out.good())
    throw std::runtime_error("error writing leaf node data");
}

} // namespace kche_tree
