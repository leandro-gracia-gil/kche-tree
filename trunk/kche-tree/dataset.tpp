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
 * \file dataset.tpp
 * \brief Template implementation for data sets containing an array of feature vectors.
 * \author Leandro Graciá Gil
 */

// Include STL strings.
#include <string>

namespace kche_tree {

// KD-Tree serialization settings.
template <typename T, unsigned int D> const uint16_t DataSet<T, D>::version[2] = { 1, 0 };

/**
 * \brief Create an empty data set.
 */
template <typename T, unsigned int D>
DataSet<T, D>::DataSet() : size_(0) {}

/**
 * \brief Create an uninitialized data set of the specified size.
 *
 * \param size Number of vectors to be contained in the set.
 */
template <typename T, unsigned int D>
DataSet<T, D>::DataSet(unsigned int size)
    : vectors_(size ? new Vector[size] : NULL),
      size_(size) {}

/**
 * \brief Create a data set object with the data of a vector array of the specified size.
 *
 * This constructor will make a copy of the provided vectors and share them whenever
 * possible between different instances of DataSet objects.
 *
 * To avoid making copies, embeed the pointer in a SharedArray object.
 * This should only be used with dynamically allocated memory.
 *
 * \param vectors Array of feature vectors to copy and use in the data set.
 * \param size Number of vectors in the array.
 */
template <typename T, unsigned int D>
DataSet<T, D>::DataSet(const Vector *vectors, unsigned int size)
    : vectors_(SharedArray<Vector>(size ? new Vector[size] : NULL)),
      size_(size) {
  if (vectors_)
    Traits<Vector>::copy_array(vectors_.get(), vectors, size);
}

/**
 * \brief Create a data set object with the data of a shared vector array of the specified size.
 *
 * This constructor makes no copies of the input array of vectors, but keeps a reference to them.
 * This reference might be released and replaced by a new shared copy by some operations
 * like modifying the vectors when shared across multiple data sets.
 *
 * \param vectors Reference-counted array of feature vectors to use in the data set.
 * \param size Number of vectors in the array.
 */
template <typename T, unsigned int D>
DataSet<T, D>::DataSet(SharedArray<Vector> vectors, unsigned int size)
    : vectors_(vectors),
      size_(size) {}

/**
 * \brief Create a permuted copy of another data set.
 *
 * Permutation will be transparent to any access outside the class based on the subscript operators.
 * For actual permutation-sensitive access, use the get_permuted methods.
 *
 * \param dataset Data set to be copied.
 * \param permutation Array describing the permutation to original vector indices.
 *        This method takes ownership of the pointer. Must be a valid permutation.
 */
template <typename T, unsigned int D>
DataSet<T, D>::DataSet(const DataSet &dataset, unsigned int *permutation)
    : vectors_(dataset.size() ? new Vector[dataset.size()] : NULL),
      permuted_to_original_(permutation),
      original_to_permuted_(permutation && dataset.size() ? new unsigned int[dataset.size()] : NULL),
      size_(dataset.size()) {
  if (!permutation)
    return;

  for (unsigned int i=0; i<size_; ++i) {
    // Equivalent to vectors_[original_to_permuted_[i]] = dataset[i], but in one pass.
    vectors_[i] = dataset[permuted_to_original_[i]];
    original_to_permuted_[permuted_to_original_[i]] = i;
  }
}

/**
 * \brief Default virtual destructor.
 */
template <typename T, unsigned int D>
DataSet<T, D>::~DataSet() {}

/**
 * \brief Reset the data set to an uninitialized version of the specified size.
 *
 * Any existing contents in the data set will be deleted.
 *
 * \param size Number of vectors to be contained in the set.
 */
template <typename T, unsigned int D>
void DataSet<T, D>::reset_to_size(unsigned int size) {
  vectors_.reset(size ? new Vector[size] : NULL);
  size_ = size;
}

/**
 * \brief Fill the dataset data using random values from a provided generator.
 *
 * \warning In the case of labeled datasets only the feature vectors are randomly initialized. Labels are not.
 *
 * \param generator Random number generator used to generate the dataset contents using its parenthesis operator.
 */
template <typename T, unsigned int D> template <typename RandomGenerator>
void DataSet<T, D>::set_random_values(RandomGenerator &generator) {
  for (unsigned int i=0; i<size_; ++i)
    for (unsigned int d=0; d<D; ++d)
      vectors_[i][d] = Traits<T>::random(generator);
}

/**
 * \brief Get the permuted version of an index.
 *
 * \note Permutation is completely transparent to the user through the subscript operator.
 *       No conversion should be required except for specific purposes.
 *
 * \param index Index referring to the original data set contents.
 * \return Permuted version of \a index if any. Returns \a index if the data set is not permuted.
 */
template <typename T, unsigned int D>
unsigned int DataSet<T, D>::get_permuted_index(unsigned int index) const {
  return original_to_permuted_ ? original_to_permuted_[index] : index;
}

/**
 * \brief Get the original non-permuted version of a permuted index.
 *
 * \note Permutation is completely transparent to the user through the subscript operator.
 *       No conversion should be required except for specific purposes.
 *
 * \param index Index referring to the permuted data set contents.
 * \return Original non-permuted version of \a index if any. Returns \a index if the data set is not permuted.
 */
template <typename T, unsigned int D>
unsigned int DataSet<T, D>::get_original_index(unsigned int index) const {
  return permuted_to_original_ ? permuted_to_original_[index] : index;
}

/**
 * \brief Access vectors of the data set in their actual positions affected by internal permutations.
 *
 * \param index Index of the vector to access. Affected by any internal permutation.
 */
template <typename T, unsigned int D>
const typename DataSet<T, D>::Vector& DataSet<T, D>::get_permuted(unsigned int index) const {
  KCHE_TREE_DCHECK(vectors_);
  KCHE_TREE_DCHECK(index < size_);
  return vectors_[index];
}

/**
 * \brief Access vectors of the data set in their actual positions affected by internal permutations.
 *
 * In order to ensure the integrity of the data, this method makes a copy
 * of the vectors in case they being shared.
 *
 * \param index Index of the vector to access. Affected by any internal permutation.
 */
template <typename T, unsigned int D>
typename DataSet<T, D>::Vector& DataSet<T, D>::get_permuted(unsigned int index) {
  KCHE_TREE_DCHECK(vectors_);
  KCHE_TREE_DCHECK(index < size_);

  if (!vectors_.unique()) {
    SharedArray<Vector> new_vectors(new Vector[size()]);
    Traits<Vector>::copy_array(new_vectors.get(), vectors_.get(), size());
    std::swap(vectors_, new_vectors);
  }

  return vectors_[index];
}

/**
 * \brief Access vectors of the data set in a way transparent to any internal permutations.
 *
 * \param index Index of the vector to access. The index is not affected by any internal permutation.
 */
template <typename T, unsigned int D>
const typename DataSet<T, D>::Vector& DataSet<T, D>::operator [] (unsigned int index) const {
  return get_permuted(original_to_permuted_ ? original_to_permuted_[index] : index);
}

/**
 * \brief Access vectors of the data set in a way transparent to any internal permutations.
 *
 * In order to ensure the integrity of the data, this method makes a copy
 * of the vectors in case they being shared.
 *
 * \param index Index of the vector to access. The index is not affected by any internal permutation.
 */
template <typename T, unsigned int D>
typename DataSet<T, D>::Vector& DataSet<T, D>::operator [] (unsigned int index) {
  return get_permuted(original_to_permuted_ ? original_to_permuted_[index] : index);
}

/**
 * \brief Check if the data set and its contents are equal to another data set.
 *
 * Comparison may be optimized if \link kche_tree::HasTrivialEqual HasTrivialEqual::value\endlink is \c true.
 */
template <typename T, unsigned int D>
bool DataSet<T, D>::operator == (const DataSet &dataset) const {
  if (size_ != dataset.size_)
    return false;
  else if (vectors_ == dataset.vectors_)
    return true;
  return Traits<Vector>::equal_arrays(vectors_.get(), dataset.vectors_.get(), size_);
}

/**
 * \brief Check if the data set and its contents are different to another data set.
 *
 * Comparison may be optimized if \link kche_tree::HasTrivialEqual HasTrivialEqual::value\endlink is \c true.
 */
template <typename T, unsigned int D>
bool DataSet<T, D>::operator != (const DataSet &dataset) const {
  return !(*this == dataset);
}

/**
 * \brief Create an iterator that goes through the specified column of the vectors in a dataset.
 */
template <typename T, unsigned int D>
DataSet<T, D>::ColumnConstIterator::ColumnConstIterator(const DataSet &dataset, unsigned int column, unsigned int row)
    : dataset_(dataset), column_(column), row_(row) {}

/**
 * \brief Create a column iterator based on the contents of another one.
 */
template <typename T, unsigned int D>
DataSet<T, D>::ColumnConstIterator::ColumnConstIterator(const ColumnConstIterator &iterator)
    : dataset_(iterator.dataset_), column_(iterator.column_), row_(iterator.row_) {}

/**
 * \brief Compares two column iterators to check if they are equal.
 */
template <typename T, unsigned int D>
bool DataSet<T, D>::ColumnConstIterator::operator == (const ColumnConstIterator &iterator) const {
  return dataset_ == iterator.dataset_ && column_ == iterator.column_ && row_ == iterator.row_;
}

/**
 * \brief Compares two column iterators to check if they are not equal.
 */
template <typename T, unsigned int D>
bool DataSet<T, D>::ColumnConstIterator::operator != (const ColumnConstIterator &iterator) const {
  return dataset_ != iterator.dataset_ || column_ != iterator.column_ || row_ != iterator.row_;
}

/**
 * \brief Move the column iterator to the next row (prefix).
 */
template <typename T, unsigned int D>
typename DataSet<T, D>::ColumnConstIterator& DataSet<T, D>::ColumnConstIterator::operator ++ () {
  ++row_;
  return *this;
}

/**
 * \brief Move the column iterator to the next row (postfix).
 */
template <typename T, unsigned int D>
typename DataSet<T, D>::ColumnConstIterator DataSet<T, D>::ColumnConstIterator::operator ++ (int) {
  ColumnConstIterator temp(*this);
  operator ++();
  return temp;
}

/**
 * \brief Move the column iterator to the previous row (prefix).
 */
template <typename T, unsigned int D>
typename DataSet<T, D>::ColumnConstIterator& DataSet<T, D>::ColumnConstIterator::operator -- () {
  --row_;
  return *this;
}

/**
 * \brief Move the column iterator to the previous row (postfix).
 */
template <typename T, unsigned int D>
typename DataSet<T, D>::ColumnConstIterator DataSet<T, D>::ColumnConstIterator::operator -- (int) {
  ColumnConstIterator temp(*this);
  operator --();
  return temp;
}

/**
 * \brief Get a non-mutable reference to the corresponding element in the data set.
 */
template <typename T, unsigned int D>
const T& DataSet<T, D>::ColumnConstIterator::operator * () {
  return dataset_[row_][column_];
}

/**
 * \brief Get a const pointer to the corresponding element in the data set.
 */
template <typename T, unsigned int D>
const T* DataSet<T, D>::ColumnConstIterator::operator -> () {
  return &dataset_[row_][column_];
}

/**
 * \brief Get an iterator to the beginning of a column in the data set.
 */
template <typename T, unsigned int D>
typename DataSet<T, D>::ColumnConstIterator DataSet<T, D>::column_begin(unsigned int column) const {
  return ColumnConstIterator(*this, column, 0);
}

/**
 * \brief Get an iterator to the end of a column in the data set.
 */
template <typename T, unsigned int D>
typename DataSet<T, D>::ColumnConstIterator DataSet<T, D>::column_end(unsigned int column) const {
  return ColumnConstIterator(*this, column, size());
}

/**
 * \brief Save the contents of the data set to the output stream.
 *
 * \param out Output stream.
 * \exception std::runtime_error In case of writing error.
 */
template <typename T, unsigned int D>
void DataSet<T, D>::serialize(std::ostream &out) const {

  // Write format version.
  kche_tree::serialize(DataSet::version, out);
  if (!out.good())
    throw std::runtime_error("error writing dataset format version");

  // Write the size of the data set.
  uint32_t size_u32 = size_;
  kche_tree::serialize(size_u32, out);
  if (!out.good())
    throw std::runtime_error("error writing the size of the data set");

  if (!size_)
    return;

  // Write the feature vectors.
  serialize_array(vectors_.get(), size_, out);
  if (!out.good())
    throw std::runtime_error("error writing the vector data");

  // Check the data set is permuted and serialize the indices if so.
  uint8_t is_permuted = permuted_to_original_ ? 1 : 0;
  kche_tree::serialize(is_permuted, out);
  if (!out.good())
    throw std::runtime_error("error writing the permutation data");

  if (is_permuted) {
    ScopedArray<uint32_t> serializable_permutation(new uint32_t[size_]);
    for (unsigned int i=0; i<size_; ++i)
      serializable_permutation[i] = permuted_to_original_[i];

    serialize_array(serializable_permutation.get(), size_, out);
    if (!out.good())
      throw std::runtime_error("error writing the permutation data");
  }
}

/**
 * \brief Deserialize a data set object from the contents of the input stream.
 *
 * \param in Input stream.
 * \param endianness Endianness of the data.
 * \exception std::runtime_error In case of reading or validation error.
 */
template <typename T, unsigned int D>
DataSet<T, D>::DataSet(std::istream &in, Endianness::Type endianness) {

  // Read format version.
  uint16_t version[2];
  kche_tree::deserialize(version, in, endianness);
  if (!in.good())
    throw std::runtime_error("error reading version data");

  // Check supported file versions.
  if (version[0] != DataSet::version[0] || version[1] != DataSet::version[1]) {
    std::string error_msg = "unsupported dataset version: required ";
    error_msg += DataSet::version[0];
    error_msg += ".";
    error_msg += DataSet::version[1];
    error_msg += ", found ";
    error_msg += version[0];
    error_msg += ".";
    error_msg += version[1];
    throw std::runtime_error(error_msg);
  }

  // Read the size of the data set.
  uint32_t size_u32;
  kche_tree::deserialize(size_u32, in, endianness);
  if (!in.good())
    throw std::runtime_error("error reading the size of the data set");

  // Resize the data set.
  reset_to_size(size_u32);

  // Stop on empty data sets.
  if (!size_)
    return;

  // Read the feature vectors.
  deserialize_array(vectors_.get(), size_, in, endianness);
  if (!in.good())
    throw std::runtime_error("error reading vector data");

  // Check if the data set is permuted.
  uint8_t is_permuted;
  kche_tree::deserialize(is_permuted, in, endianness);

  if (!in.good())
    throw std::runtime_error("error reading permutation data");

  // If it is, allocate and read the permutation array.
  if (is_permuted) {
    ScopedArray<uint32_t> serialized_permutation(new uint32_t[size_]);
    deserialize_array(serialized_permutation.get(), size_, in, endianness);

    if (!in.good())
      throw std::runtime_error("error reading permutation data");

    // Get the direct and inverse permutations.
    permuted_to_original_.reset(new unsigned int[size_]);
    original_to_permuted_.reset(new unsigned int[size_]);
    memset(original_to_permuted_.get(), 0xFF, size_ * sizeof(unsigned int));

    for (unsigned int i=0; i<size_; ++i) {
      permuted_to_original_[i] = serialized_permutation[i];

      // Verify the permutation data.
      if (permuted_to_original_[i] >= size_ || original_to_permuted_[permuted_to_original_[i]] != 0xFFFFFFFFU)
        throw std::runtime_error("invalid data set permutation data");
      original_to_permuted_[permuted_to_original_[i]] = i;
    }
  }
}

/**
 * \brief Swap the contents of the data set with some other.
 *
 * Used as part of the deserialization process.
 *
 * \param dataset Data set to swap contents with.
 */
template <typename T, unsigned int D>
void DataSet<T, D>::swap(DataSet &dataset) {
  std::swap(size_, dataset.size_);
  vectors_.swap(dataset.vectors_);
  permuted_to_original_.swap(dataset.permuted_to_original_);
  original_to_permuted_.swap(dataset.original_to_permuted_);
}

} // namespace kche_tree
