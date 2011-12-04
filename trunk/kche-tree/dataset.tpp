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
 * \file dataset.tpp
 * \brief Template implementation for data sets containing an array of feature vectors.
 * \author Leandro Graciá Gil
 */

// Include STL strings.
#include <string>

namespace kche_tree {

// KD-Tree serialization settings.
template <typename T, const unsigned int D> const uint16_t DataSet<T, D>::version[2] = { 1, 0 };

/**
 * Create an empty data set with no size and no vectors on it.
 */
template <typename T, const unsigned int D>
DataSet<T, D>::DataSet() : size_(0) {}

/**
 * No contents of the data set will be initialized.
 *
 * \param size Number of vectors to be contained in the set.
 */
template <typename T, const unsigned int D>
DataSet<T, D>::DataSet(unsigned int size)
  : vectors_(size ? new VectorType[size] : NULL),
    size_(size) {}

/**
 * Since this constructor makes the object to depend on external data,
 * it requires it to be a reference-counted pointer to ensure its lifetime.
 *
 * \param vectors Reference-counted array of feature vectors to use in the data set.
 * \param size Number of vectors in the array.
 */
template <typename T, const unsigned int D>
DataSet<T, D>::DataSet(SharedArray<VectorType> vectors, unsigned int size)
  : vectors_(vectors),
    size_(size) {}

/**
 * Release any existing contents in the data set and leave an uninitialized set of the requested size.
 *
 * \param size Number of vectors to be contained in the set.
 */
template <typename T, const unsigned int D>
void DataSet<T, D>::reset_to_size(unsigned int size) {
  vectors_.reset(size ? new VectorType[size] : NULL);
  size_ = size;
}

/**
 * Release any existing contents and generate a random dataset of the specified size using a provided generator.
 *
 * \param size Number of vectors to be contained in the set.
 * \param generator Random number generator used to generate the dataset contents by means of its parenthesis operator.
 */
template <typename T, const unsigned int D> template <typename RandomGeneratorType>
void DataSet<T, D>::reset_to_random(unsigned int size, RandomGeneratorType &generator) {
  reset_to_size(size);
  for (unsigned int i=0; i<size; ++i)
    for (unsigned int d=0; d<D; ++d)
      vectors_[i][d] = Traits<T>::random(generator);
}

/**
 * Access the vectors in the data set, but make a copy of them if they
 * are shared with anything else to ensure the integrity of the data.
 */
template <typename T, const unsigned int D>
typename DataSet<T, D>::VectorType &DataSet<T, D>::operator [] (unsigned int index) {
  KCHE_TREE_DCHECK(vectors_);

  if (!vectors_.unique()) {
    SharedArray<VectorType> new_vectors(new VectorType[size_]);
    Traits<VectorType>::copy_array(new_vectors.get(), vectors_.get(), size_);
    swap(vectors_, new_vectors);
  }

  return vectors_[index];
}

/**
 * Check if the contents of the data set are equal to some other's.
 */
template <typename T, const unsigned int D>
bool DataSet<T, D>::operator == (const DataSet& dataset) const {
  if (size_ != dataset.size_)
    return false;
  else if (vectors_ == dataset.vectors_)
    return true;
  return Traits<VectorType>::equal_arrays(vectors_.get(), dataset.vectors_.get(), size_);
}

/**
 * Check if the contents of the data set are different to some other's.
 */
template <typename T, const unsigned int D>
bool DataSet<T, D>::operator != (const DataSet& dataset) const {
  return !(*this == dataset);
}

/**
 * \brief Save the contents of the data set to the output stream.
 *
 * \param out Output stream.
 * \param dataset DataSet to be serialized.
 * \exception std::runtime_error In case of writing error.
 */
template <typename T, const unsigned int D>
std::ostream& operator << (std::ostream& out, const DataSet<T, D> &dataset) {

  // Type aliases.
  typedef DataSet<T, D> DataSetType;
  typedef typename DataSetType::VectorType VectorType;

  // Check the state of the output stream.
  if (!out.good())
    throw std::runtime_error("output stream not ready");

  // Serialize the endianness being used.
  uint8_t endianness_raw = Endianness::endianness();
  serialize(endianness_raw, out);
  if (!out.good())
    throw std::runtime_error("error writing endianness information");

  // Write format version.
  serialize(DataSetType::version, out);
  if (!out.good())
    throw std::runtime_error("error writing dataset format version");

  // Serialize the type of the data set into the stream.
  Traits<DataSetType>::serialize_type(out);
  if (!out.good())
    throw std::runtime_error("error serializing the data set type information");

  // Serialize the vector type into the stream. This implicitely serializes both the type T and the number of dimensions.
  Traits<VectorType>::serialize_type(out);
  if (!out.good())
    throw std::runtime_error("error serializing the vector type information");

  // Write the size of the data set.
  serialize(dataset.size_, out);
  if (!out.good())
    throw std::runtime_error("error writing the size of the data set");

  // Write the feature vectors.
  serialize_array(dataset.vectors_.get(), dataset.size_, out);
  if (!out.good())
    throw std::runtime_error("error writing the vector data");

  return out;
}

/**
 * \brief Load the contents of the input stream from the data set.
 * The original contents of the \a dataset object are not modified in case of error.
 *
 * \param in Input stream.
 * \param dataset DataSet to be deserialized.
 * \exception std::runtime_error In case of reading or validation error.
 */
template <typename T, const unsigned int D>
std::istream& operator >> (std::istream& in, DataSet<T, D> &dataset) {

  // Type aliases.
  typedef DataSet<T, D> DataSetType;
  typedef typename DataSetType::VectorType VectorType;

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

  // Read format version.
  uint16_t version[2];
  deserialize(version, in, endianness);
  if (!in.good())
    throw std::runtime_error("error reading version data");

  // Check supported file versions.
  if (version[0] != DataSetType::version[0] || version[1] != DataSetType::version[1]) {
    std::string error_msg = "unsupported dataset version: required ";
    error_msg += DataSetType::version[0];
    error_msg += ".";
    error_msg += DataSetType::version[1];
    error_msg += ", found ";
    error_msg += version[0];
    error_msg += ".";
    error_msg += version[1];
    throw std::runtime_error(error_msg);
  }

  // Check the type of the data set from the stream. Will throw std::runtime_error if it doesn't match.
  // This will implicitely check both the type of T and the number of dimensions.
  Traits<DataSetType>::check_serialized_type(in, endianness);

  // Check the serialized type. Will throw std::runtime_error in case it doesn't match.
  Traits<VectorType>::check_serialized_type(in, endianness);

  // Read the size of the data set.
  uint32_t size;
  deserialize(size, in, endianness);
  if (!in.good())
    throw std::runtime_error("error reading the size of the data set");

  // Check number of vectors.
  if (!size)
    throw std::runtime_error("invalid size of the data set");

  // Allocate memory and read the feature vectors.
  SharedArray<VectorType> vectors(new VectorType[size]);
  deserialize_array(vectors.get(), size, in, endianness);
  if (!in.good())
    throw std::runtime_error("error reading the vectors data");

  // Set the data to the object.
  dataset.size_ = size;
  dataset.vectors_ = vectors;

  return in;
}

} // namespace kche_tree
