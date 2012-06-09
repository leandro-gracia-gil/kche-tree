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
 * \file labeled_dataset.tpp
 * \brief Template implementation for data sets containing an array of labeled feature vectors.
 * \author Leandro Graciá Gil
 */

namespace kche_tree {

/**
 * \brief Create an empty labeled data set.
 */
template <typename T, unsigned int D, typename L>
LabeledDataSet<T, D, L>::LabeledDataSet() {}

/**
 * \brief Create an uninitialized labeled data set of the specified size.
 *
 * \param size Number of vectors to be contained in the set.
 */
template <typename T, unsigned int D, typename L>
LabeledDataSet<T, D, L>::LabeledDataSet(unsigned int size)
    : kche_tree::DataSet<T, D>(size),
      labels_(SharedArray<Label>(size ? new Label[size] : NULL)) {}

/**
 * \brief Create a labeled data set object with the data of vector and label arrays of the specified size.
 *
 * This constructor will make a copy of the provided vectors and labels and share
 * them whenever possible between different instances of LabeledDataSet objects.
 *
 * To avoid making copies, embeed the pointer in a SharedArray object.
 * This should only be used with dynamically allocated memory.
 *
 * \param vectors Array of feature vectors to copy and use in the data set.
 * \param labels Array of labels associated to each of the feature vectors.
 * \param size Number of vectors in the array.
 */
template <typename T, unsigned int D, typename L>
LabeledDataSet<T, D, L>::LabeledDataSet(const Vector *vectors, const Label *labels, unsigned int size)
    : kche_tree::DataSet<T, D>(vectors, size),
      labels_(SharedArray<Label>(size ? new Label[size] : NULL)) {
  if (labels_)
    Traits<Label>::copy_array(labels_.get(), labels, size);
}

/**
 * \brief Create a labeled data set object with the data of shared vector and label arrays of the specified size.
 *
 * This constructor makes no copies of the input array of vectors and labels, but keeps references to them.
 * This reference might be released and replaced by a new shared copy by some operations
 * like modifying the vectors or labels when shared across multiple data sets.
 *
 * \param vectors Reference-counted array of feature vectors to use in the data set.
 * \param labels Reference-counted array of labels associated to each of the feature vectors.
 * \param size Number of vectors in the array.
 */
template <typename T, unsigned int D, typename L>
LabeledDataSet<T, D, L>::LabeledDataSet(SharedArray<Vector> vectors, SharedArray<Label> labels, unsigned int size)
  : kche_tree::DataSet<T, D>(vectors, size),
    labels_(labels) {}

/**
 * \brief Create a permuted copy of another labeled data set.
 *
 * Only vector data is actually permuted, labels are not.
 *
 * Permutation will be transparent to any access outside the class based on the subscript operators.
 * For actual permutation-sensitive access, use the get_permuted methods.
 *
 * \param dataset Data set to be copied.
 * \param permutation Array describing the permutation to original vector indices.
 *        This method takes ownership of the pointer. Must be a valid permutation.
 */
template <typename T, unsigned int D, typename L>
LabeledDataSet<T, D, L>::LabeledDataSet(const LabeledDataSet &dataset, unsigned int *permutation)
    : kche_tree::DataSet<T, D>(dataset, permutation),
      labels_(dataset.labels_) {}

/**
 * \brief Default virtual destructor.
 */
template <typename T, unsigned int D, typename L>
LabeledDataSet<T, D, L>::~LabeledDataSet() {}

/**
 * \brief Reset the labeled data set to an uninitialized version of the specified size.
 *
 * Any existing contents in the data set including labels will be deleted.
 *
 * \param size Number of vectors to be contained in the set.
 */
template <typename T, unsigned int D, typename L>
void LabeledDataSet<T, D, L>::reset_to_size(unsigned int size) {
  kche_tree::DataSet<T, D>::reset_to_size(size);
  labels_.reset(size ? new Label[size] : NULL);
  // TODO: if (HasTrivialConstructor<T>::value && is_pod<T>::value)
    for (unsigned int i=0; i<size; ++i)
      labels_[i] = Label();
}

/**
 * \brief Retrieve the label associated to an index (const version).
 */
template <typename T, unsigned int D, typename L>
const L& LabeledDataSet<T, D, L>::label(unsigned int index) const {
  return labels_[index];
}

/**
 * \brief Retrieve the label associated to an index.
 *
 * Creates a separate copy of the labels if shared.
 */
template <typename T, unsigned int D, typename L>
L& LabeledDataSet<T, D, L>::label(unsigned int index) {

  // Make a separate copy if labels are being shared with something else.
  // Note: unlike vectors, labels are not permuted in the array.
  if (!labels_.unique()) {
    SharedArray<Label> new_labels(new Label[this->size()]);
    Traits<Label>::copy_array(new_labels.get(), labels_.get(), this->size());
    std::swap(labels_, new_labels);
  }

  return labels_[index];
}

/**
 * \brief Check if the data set, its contents and its labels are equal to another data set.
 *
 * Comparison may be optimized if \link kche_tree::HasTrivialEqual HasTrivialEqual::value\endlink is \c true.
 */
template <typename T, unsigned int D, typename L>
bool LabeledDataSet<T, D, L>::operator == (const LabeledDataSet &dataset) const {
  if (!DataSet::operator == (dataset))
    return false;
  if (labels_ == dataset.labels_)
    return true;
  return Traits<Label>::equal_arrays(labels_.get(), dataset.labels_.get(), this->size());
}

/**
 * \brief Check if the data set, its contents and its labels are different to another data set.
 *
 * Comparison may be optimized if \link kche_tree::HasTrivialEqual HasTrivialEqual::value\endlink is \c true.
 */
template <typename T, unsigned int D, typename L>
bool LabeledDataSet<T, D, L>::operator != (const LabeledDataSet &dataset) const {
  return !(*this == dataset);
}

/**
 * \brief Check if the contents of the data set are equal to another one. Labels are discarded.
 *
 * Comparison may be optimized if \link kche_tree::HasTrivialEqual HasTrivialEqual::value\endlink is \c true.
 */
template <typename T, unsigned int D, typename L>
bool LabeledDataSet<T, D, L>::operator == (const DataSet &dataset) const {
  return DataSet::operator == (dataset);
}

/**
 * \brief Check if the contents of the data set are different to another one. Labels are discarded.
 *
 * Comparison may be optimized if \link kche_tree::HasTrivialEqual HasTrivialEqual::value\endlink is \c true.
 */
template <typename T, unsigned int D, typename L>
bool LabeledDataSet<T, D, L>::operator != (const DataSet &dataset) const {
  return DataSet::operator != (dataset);
}

/**
 * \brief Save the contents of the data set to the output stream.
 *
 * \param out Output stream.
 * \exception std::runtime_error In case of writing error.
 */
template <typename T, unsigned int D, typename L>
void LabeledDataSet<T, D, L>::serialize(std::ostream &out) const {

  // Serialize the base data set.
  DataSet::serialize(out);

  // Check the state of the output stream.
  if (!out.good())
    throw std::runtime_error("output stream not ready");

  if (!this->size_)
    return;

  // Write the labels.
  KCHE_TREE_DCHECK(labels_);
  serialize_array(labels_.get(), this->size_, out);
  if (!out.good())
    throw std::runtime_error("error writing the label data");
}

/**
 * \brief Deserialize a labeled data set object from the contents of the input stream.
 *
 * \param in Input stream.
 * \param endianness Endianness of the data.
 * \exception std::runtime_error In case of reading or validation error.
 */
template <typename T, unsigned int D, typename L>
LabeledDataSet<T, D, L>::LabeledDataSet(std::istream &in, Endianness::Type endianness)
    : DataSet(in, endianness) {

  // Check the state of the input stream after deserializing the base data set.
  if (!in.good())
    throw std::runtime_error("error deserializing base data set");

  if (!this->size_)
    return;

  // Allocate and deserialize the labels.
  labels_.reset(this->size_ ? new Label[this->size_] : NULL);
  deserialize_array(labels_.get(), this->size_, in, endianness);
  if (!in.good())
    throw std::runtime_error("error reading the labels data");

  return;
}

/**
 * \brief Swap the contents of the data set with some other.
 *
 * Used as part of the deserialization process.
 *
 * \param dataset Labeled data set to swap contents with.
 */
template <typename T, unsigned int D, typename L>
void LabeledDataSet<T, D, L>::swap(LabeledDataSet &dataset) {
  DataSet::swap(dataset);
  labels_.swap(dataset.labels_);
}

/// Desambiguation for operator <<.
template <typename T, unsigned int D, typename Label>
std::ostream& operator << (std::ostream &out, const LabeledDataSet<T, D, Label> &dataset) {
  return out << static_cast<const Serializable<LabeledDataSet<T, D, Label> > &>(dataset);
}

/// Desambiguation for operator >>.
template <typename T, unsigned int D, typename Label>
std::istream& operator >> (std::istream &in, LabeledDataSet<T, D, Label> &dataset) {
  return in >> static_cast<Serializable<LabeledDataSet<T, D, Label> > &>(dataset);
}

} // namespace kche_tree
