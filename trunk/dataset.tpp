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

// Include STL strings and auto_ptr.
#include <memory>
#include <string>

namespace kche_tree {

/**
 * \brief Create an empty data set with no size and no vectors on it.
 */
template <typename T, const unsigned int D>
DataSet<T, D>::DataSet()
  : vectors_(NULL),
    size_(0),
    ptr_owner_(false) {}

/**
 * \brief Create a data set of the specified size.
 * No contents of the data set will be initialized.
 *
 * \param size Number of vectors to be contained in the set.
 */
template <typename T, const unsigned int D>
DataSet<T, D>::DataSet(unsigned int size)
  : vectors_(size ? new Vector<T, D>[size] : NULL),
    size_(size),
    ptr_owner_(true) {
}

/**
 * \brief Create a data set using an external array of vectors.
 *
 * This method takes no ownership of the external data. The user is responsible
 * of handling its lifetime and making it accesible to the library.
 *
 * \param vectors Array of feature vectors to use in the data set.
 * \param size Number of vectors in the array.
 */
template <typename T, const unsigned int D>
DataSet<T, D>::DataSet(Vector<T, D> *vectors, unsigned int size)
  : vectors_(vectors),
    size_(size),
    ptr_owner_(false) {
}

/**
 * Release any data owned by the object.
 */
template <typename T, const unsigned int D>
DataSet<T, D>::~DataSet() {
  if (ptr_owner_)
    delete []vectors_;
}

/**
 * Create a new data set with the contents of another one.
 * This method makes a copy of the data if any and takes ownership of it.
 *
 * \param dataset Dataset to be copied.
 */
template <typename T, const unsigned int D>
DataSet<T, D>::DataSet(const DataSet &dataset)
  : vectors_(dataset.size_ ? new Vector<T, D>[dataset.size_] : NULL),
    size_(dataset.size_),
    ptr_owner_(true) {

  if (dataset.size_)
    copy_array(vectors_, dataset.vectors_, dataset.size_);
}

/**
 * Copy the contents of an existing data set.
 * This method makes a copy of the data if any and takes ownership of it.
 *
 * \param dataset Dataset to be copied.
 */
template <typename T, const unsigned int D>
DataSet<T, D> &DataSet<T, D>::operator = (const DataSet &dataset) {

  if (ptr_owner_)
    delete []vectors_;

  size_ = dataset.size_;
  ptr_owner_ = true;

  if (dataset.size_)
    copy_array(vectors_, dataset.vectors_, dataset.size_);
  else
    vectors_ = NULL;

  return *this;
}

/**
 * \brief Reset the data set to an uninitialized version of the specified size.
 *
 * Release any existing contents in the data set and leave an uninitialized set of the requested size.
 *
 * \param size Number of vectors to be contained in the set.
 */
template <typename T, const unsigned int D>
void DataSet<T, D>::reset_to_size(unsigned int size) {

  // Release any existing owned data.
  if (ptr_owner_)
    delete []vectors_;

  // Allocate an uninitialized set of vectors of the specified size.
  vectors_ = size ? new Vector<T, D>[size] : NULL;
  size_ = size;
  ptr_owner_ = true;
}

/**
 * Check if the contents of the data set are equal to some other's.
 */
template <typename T, const unsigned int D>
bool DataSet<T, D>::operator == (const DataSet& dataset) const {
  if (size_ != dataset.size_)
    return false;
  return equal_arrays(vectors_, dataset.vectors_, size_);
}

/**
 * Check if the contents of the data set are different to some other's.
 */
template <typename T, const unsigned int D>
bool DataSet<T, D>::operator != (const DataSet& dataset) const {
  if (size_ != dataset.size_)
    return false;
  return !equal_arrays(vectors_, dataset.vectors_, size_);
}

/**
 * Save the current data set to a n output stream.
 */
template <typename T, const unsigned int D>
std::ostream& operator << (std::ostream& out, const DataSet<T, D> &dataset) {

  // Check the state of the output stream.
  if (!out.good())
    throw std::runtime_error("output stream not ready");

  // Write the size of the data set.
  out.write(reinterpret_cast<const char *>(&dataset.size_), sizeof(unsigned int));
  if (!out.good())
    throw std::runtime_error("error writing the size of the data set");

  // Write name of the type T with its length.
  unsigned short name_length = strlen(typeid(T).name());
  out.write(reinterpret_cast<const char *>(&name_length), sizeof(unsigned short));
  out.write(typeid(T).name(), name_length);
  if (!out.good())
    throw std::runtime_error("error writing the type of the vectors");

  // Write the number of dimensions of the vectors.
  unsigned int dimensions = D;
  out.write(reinterpret_cast<const char *>(&dimensions), sizeof(unsigned int));
  if (!out.good())
    throw std::runtime_error("error writing the number of dimensions");

  // Write the feature vectors.
  out.write(reinterpret_cast<const char *>(&dataset.vectors_), dataset.size_ * sizeof(Vector<T, D>));
  if (!out.good())
    throw std::runtime_error("error writing the vector data");

  return out;
}

/**
 * Load the contents of the input stream to the data set.
 * Any existing data will be released.
 */
template <typename T, const unsigned int D>
std::istream& operator >> (std::istream& in, DataSet<T, D> &dataset) {

  // Check the state of the input stream.
  if (!in.good())
    throw std::runtime_error("input stream not ready");

  // Read the size of the data set.
  unsigned int size;
  in.read(reinterpret_cast<char *>(&size), sizeof(unsigned int));
  if (!in.good())
    throw std::runtime_error("error reading the size of the data set");

  // Check number of vectors.
  if (!size)
    throw std::runtime_error("invalid size of the data set");

  // Read type name length.
  unsigned short name_length;
  in.read(reinterpret_cast<char *>(&name_length), sizeof(unsigned short));
  if (!in.good())
    throw std::runtime_error("error reading type name length data");

  // Read type name.
  std::auto_ptr<char> type_name(new char[name_length + 1]);
  assert(type_name.get());
  in.read(type_name.get(), name_length);
  if (!in.good())
    throw std::runtime_error("error reading type name");
  type_name.get()[name_length] = '\0';

  // Check type name.
  // WARNING: The value returned by typeid::name() is implementation-dependent.
  //          There is a possibility of problems when porting data between different platforms.
  if (strcmp(typeid(T).name(), type_name.get())) {
    std::string error_msg = "kd-tree type doesn't match: found ";
    error_msg += type_name.get();
    error_msg += ", expected ";
    error_msg += typeid(T).name();
    throw std::runtime_error(error_msg);
  }

  // Read the number of dimensions of the vectors.
  unsigned int dimensions;
  in.read(reinterpret_cast<char *>(&dimensions), sizeof(unsigned int));
  if (!in.good())
    throw std::runtime_error("error reading the number of dimensions");

  // Check number of dimensions.
  if (dimensions != D)
    throw std::runtime_error("non-compatible number of dimensions");

  // Allocate memory and read the feature vectors.
  Vector<T, D> *vectors = new Vector<T, D>[size];
  in.read(reinterpret_cast<char *>(vectors), size * sizeof(Vector<T, D>));
  if (!in.good())
    throw std::runtime_error("error reading the vector data");

  // Release any existing data.
  if (dataset.ptr_owner_)
    delete []dataset.vectors_;

  // Set the data to the object.
  dataset.size_ = size;
  dataset.vectors_ = vectors;
  dataset.ptr_owner_ = true;

  return in;
}

} // namespace kche_tree
