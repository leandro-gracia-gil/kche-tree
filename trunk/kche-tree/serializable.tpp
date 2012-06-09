/***************************************************************************
 *   Copyright (C) 2012 by Leandro Graciá Gil                              *
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
 * \file serializable.tpp
 * \brief Base class for kche-tree serializable objects.
 * \author Leandro Graciá Gil
 */

#include "serializable.h"

#include <cstring>
#include <typeinfo>

#include "scoped_ptr.h"

// Demangling functions for the GNU C++ compiler.
#ifdef __GNUG__
#include <cxxabi.h>
#endif

namespace kche_tree {

/**
 * \brief Get the name of the dynamic type of the object.
 *
 * For platform compatibility purposes the returned name is demangled if available.
 *
 * \return The name of the dynamic type of the object, possibly demangled.
 */
template <typename T>
const char *Serializable<T>::type_name() const {
  static const char *typeid_name = typeid(T).name();
  #ifdef __GNUG__
  static int status;
  static const char *realname = abi::__cxa_demangle(typeid_name, 0, 0, &status);
  if (!status)
    return realname;
  #endif
  return typeid_name;
}

/**
 * \brief Check the serialized type information from an input stream.
 *
 * Since the type itself cannot be deserialized (especially for templates)
 * this method only verifies that the type names match.
 *
 * \warning Unfortunately, the type name returned by the typeid operator in C++ is platform-dependent.
 * In order to reduce problems across platforms the serialized type name is the demangled version (if supported)
 * of typeid::name(). However, there is a possibility of incompatible files when porting data between different platforms.
 *
 * \param in Input stream.
 * \param endianness Endianness of the serialized data. Defaults to host's endianness.
 * \exception std::runtime_error Thrown in case of error while deserializing the type name or if the names doesn't match.
 */
template <typename T>
void Serializable<T>::check_serialized_type(std::istream &in, Endianness::Type endianness) const {

  // Read type name length.
  uint32_t name_length;
  deserialize(name_length, in, endianness);
  if (!in.good())
    throw std::runtime_error("error reading type name length data");

  // Read type name.
  ScopedArray<char> serialized_type_name(new char[name_length + 1]);
  deserialize_array(serialized_type_name.get(), name_length, in);
  if (!in.good())
    throw std::runtime_error("error reading type name");
  serialized_type_name[name_length] = '\0';

  // Check type name.
  const char *name = type_name();
  if (strcmp(name, serialized_type_name.get())) {
    std::string error_msg = "type doesn't match: found ";
    error_msg += serialized_type_name.get();
    error_msg += ", expected ";
    error_msg += name;
    throw std::runtime_error(error_msg);
  }
}

/**
 * \brief Serialize the name of the dynamic type of the object into a stream.
 *
 * The serialized name will be demangled if available.
 */
template <typename T>
void Serializable<T>::serialize_type(std::ostream &out) const {
  const char *name = type_name();
  uint32_t name_length = strlen(name);
  kche_tree::serialize(name_length, out);
  kche_tree::serialize_array(name, name_length, out);
}

/**
 * \brief Serialize the contents of an object into the output stream.
 *
 * Unlike \link kche_tree::Serializable<T>::serialize serialize \endlink, this method
 * serializes the endianess and the type name of the object.
 *
 * \note This method is safe to be used with derived serializable objects as the type
 * is resolved dynamically.
 *
 * \param out Output stream.
 * \param serializable Object to be serialized.
 * \exception std::runtime_error In case of serialization error.
 */
template <typename T>
std::ostream& operator << (std::ostream& out, const Serializable<T> &serializable) {

  // Check the state of the output stream.
  if (!out.good())
    throw std::runtime_error("output stream not ready");

  // Serialize the endianness being used (same as host).
  Endianness::serialize(out);

  // The template type defines the actual derived type.
  const T &serializable_specific = static_cast<const T&>(serializable);

  // Serialize the dynamic type of the object.
  serializable_specific.serialize_type(out);

  // Serialize the object.
  serializable_specific.serialize(out);

  return out;
}

/**
 * \brief Load the contents of the input stream from the data set.
 * The original contents of the \a dataset object are not modified in case of error.
 *
 * \note Because the full dynamic data set type cannot be recovered from the deserialization
 * (only its possibly demangled name) this method cannot read serialized derived types
 * like labeled data sets and store them in the \a dataset parameter. Consequently, this
 * method should only be used to read actual DataSet objects. To deserialize derived objects
 * their respective versions of this operator should be used.
 *
 * \param in Input stream.
 * \param serializable Object to be deserialized.
 * \exception std::runtime_error In case of reading or validation error.
 */
template <typename T>
std::istream& operator >> (std::istream &in, Serializable<T> &serializable) {

  // Check the state of the input stream.
  if (!in.good())
    throw std::runtime_error("input stream not ready");

  // Read and validate format endianness.
  Endianness::Type endianness = Endianness::deserialize(in);

  // The template type defines the actual derived type.
  T &serializable_specific = static_cast<T&>(serializable);

  // Check the type of the stream. Will throw std::runtime_error if it doesn't match.
  serializable_specific.check_serialized_type(in, endianness);

  // Deserialize into a temporary object. Throws and exception in case of error.
  ScopedPtr<T> temp(new T(in, endianness));

  // Swap the contents with the temporary object.
  // This prevents partial deserializations in case of error.
  if (temp)
    temp->swap(serializable_specific);

  // Return the input stream.
  return in;
}

} // namespace kche_tree
