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
 * \file traits.tpp
 * \brief Template implementations for type traits and their basic operations.
 * \author Leandro Graciá Gil
 */

namespace kche_tree {

/**
 * \brief Compare the contents of two arrays of the given size.
 *
 * \param p1 Pointer to the first array.
 * \param p2 Pointer to the second array.
 * \param size Size of both arrays.
 * \return \c true if equal, \c false otherwise.
 */
template <typename T>
bool EqualTraits<T, true>::equal_arrays(const T *p1, const T *p2, unsigned int size) {
  return memcmp(p1, p2, size * sizeof(T)) == 0;
}

/**
 * \brief Compare the contents of two arrays of the given size.
 *
 * \param p1 Pointer to the first array.
 * \param p2 Pointer to the second array.
 * \param size Size of both arrays.
 * \return \c true if equal, \c false otherwise.
 */
template <typename T>
bool EqualTraits<T, false>::equal_arrays(const T *p1, const T *p2, unsigned int size) {
  return std::equal(p1, p1 + size, p2);
}

/**
 * \brief Copy the contents of one array to another.
 * Arrays are assumed to not overlap.
 *
 * \param dest Pointer to the destination array.
 * \param source Pointer to the source array.
 * \param size Size of both arrays.
 */
template <typename T>
void CopyTraits<T, true>::copy_array(T *dest, const T *source, unsigned int size) {
  memcpy(dest, source, size * sizeof(T));
}

/**
 * \brief Copy the contents of one array to another.
 * Arrays are assumed to not overlap.
 *
 * \param dest Pointer to the destination array.
 * \param source Pointer to the source array.
 * \param size Size of both arrays.
 */
template <typename T>
void CopyTraits<T, false>::copy_array(T *dest, const T *source, unsigned int size) {
  std::copy(source, source + size, dest);
}

/**
 * \brief Serialize an element into the provided stream.
 * This version is optimized to write raw memory values.
 *
 * \note Data is serialized with the same endianness as the local host.
 * \param element Element to be written.
 * \param out Output stream.
 */
template <typename T>
void SerializationTraits<T, true>::serialize(const T &element, std::ostream &out) {
  out.write(reinterpret_cast<const char *>(&element), sizeof(T));
}

/**
 * \brief Serialize an array into the provided stream.
 * This version is optimized to write raw memory values.
 *
 * \note Data is serialized with the same endianness as the local host.
 * \param array Array to be written.
 * \param size Number of elements in \a array.
 * \param out Output stream.
 */
template <typename T>
void SerializationTraits<T, true>::serialize_array(const T *array, unsigned int size, std::ostream &out) {
  out.write(reinterpret_cast<const char *>(array), size * sizeof(T));
}

/**
 * \brief Deserialize an element from the provided stream.
 * This version is optimized to read raw memory values.
 * Endianness is corrected to match host's if required.
 *
 * \param element Element to be read.
 * \param in Input stream.
 * \param endianness Endianness of the serialized data. Defaults to host's endianness.
 */
template <typename T>
void SerializationTraits<T, true>::deserialize(T &element, std::istream &in, Endianness::Type endianness) {
  in.read(reinterpret_cast<char *>(&element), sizeof(T));
  if (sizeof(T) > 1 && endianness != Endianness::endianness())
    Traits<T>::swap_endianness(element);
}

/**
 * \brief Deserialize an array from the provided stream.
 * This version is optimized to read raw memory values.
 * Endianness is corrected to match host's if required.
 *
 * \param array Array to be read.
 * \param size Number of elements in \a array.
 * \param in Input stream.
 * \param endianness Endianness of the serialized data. Defaults to host's endianness.
 */
template <typename T>
void SerializationTraits<T, true>::deserialize_array(T *array, unsigned int size, std::istream &in, Endianness::Type endianness) {
  in.read(reinterpret_cast<char *>(array), size * sizeof(T));
  if (sizeof(T) > 1 && endianness != Endianness::endianness())
    for (unsigned int i=0; i<size; ++i)
      Traits<T>::swap_endianness(array[i]);
}

/**
 * \brief Serialize an element into the provided stream.
 * This version makes use of the << operator.
 *
 * \note Data is serialized with the same endianness as the local host.
 * \param element Element to be written.
 * \param out Output stream.
 */
template <typename T>
void SerializationTraits<T, false>::serialize(const T &element, std::ostream &out) {
  out << element;
}

/**
 * \brief Serialize an array into the provided stream.
 * This version makes use of the << operator.
 *
 * \note Data is serialized with the same endianness as the local host.
 * \param array Array to be written.
 * \param size Number of elements in \a array.
 * \param out Output stream.
 */
template <typename T>
void SerializationTraits<T, false>::serialize_array(const T *array, unsigned int size, std::ostream &out) {
  for (unsigned int i=0; i<size; ++i)
    serialize(array[i], out);
}

/**
 * \brief Deserialize an element from the provided stream.
 * Endianness is corrected to match host's if required.
 * This version makes use of the >> operator.
 *
 * \param element Element to be read.
 * \param in Input stream.
 * \param endianness Endianness of the serialized data. Defaults to host's endianness.
 */
template <typename T>
void SerializationTraits<T, false>::deserialize(T &element, std::istream &in, Endianness::Type endianness) {
  element = T(in, endianness);
}

/**
 * \brief Deserialize an array from the provided stream.
 * Endianness is corrected to match host's if required.
 * This version makes use of the >> operator.
 *
 * \param array Array to be read.
 * \param size Number of elements in \a array.
 * \param in Input stream.
 * \param endianness Endianness of the serialized data. Defaults to host's endianness.
 */
template <typename T>
void SerializationTraits<T, false>::deserialize_array(T *array, unsigned int size, std::istream &in, Endianness::Type endianness) {
  for (unsigned int i=0; i<size; ++i)
    deserialize(array[i], in, endianness);
}

/**
 * \brief Convenience function to serialize an element.
 * Will make use of optimizations if possible.
 *
 * \note Data is serialized with the same endianness as the local host.
 * \param element Element to be written.
 * \param out Output stream.
 */
template <typename T>
void serialize(const T &element, std::ostream &out) {
  SerializationTraits<T>::serialize(element, out);
}

/**
 * \brief Convenience function to serialize an array type.
 *
 * \note Data is serialized with the same endianness as the local host.
 * \param array Array to be written.
 * \param out Output stream.
 */
template <typename T, const size_t Size>
void serialize(const T (&array)[Size], std::ostream &out) {
  SerializationTraits<T>::serialize_array(array, Size, out);
}

/**
 * \brief Convenience function to serialize an element.
 * Will make use of optimizations if possible.
 *
 * \note Data is serialized with the same endianness as the local host.
 * \param array Array to be written.
 * \param size Number of elements in \a array.
 * \param out Output stream.
 */
template <typename T>
void serialize_array(const T *array, unsigned int size, std::ostream &out) {
  SerializationTraits<T>::serialize_array(array, size, out);
}

/**
 * \brief Convenience function to deserialize an element.
 * Endianness is corrected to match host's if required.
 * Will make use of optimizations if possible.
 *
 * \param element Element to be read.
 * \param in Input stream.
 * \param endianness Endianness of the serialized data. Defaults to host's endianness.
 */
template <typename T>
void deserialize(T &element, std::istream &in, Endianness::Type endianness) {
  SerializationTraits<T>::deserialize(element, in, endianness);
}

/**
 * \brief Convenience function to deserialize an array type.
 * Endianness is corrected to match host's if required.
 *
 * \param array Array to be read.
 * \param in Input stream.
 * \param endianness Endianness of the serialized data. Defaults to host's endianness.
 */
template <typename T, const size_t Size>
void deserialize(T (&array)[Size], std::istream &in, Endianness::Type endianness) {
  SerializationTraits<T>::deserialize_array(array, Size, in, endianness);
}

/**
 * \brief Convenience function to deserialize an array.
 * Endianness is corrected to match host's if required.
 * Will make use of optimizations if possible.
 *
 * \param array Array to be read.
 * \param size Number of elements in \a array.
 * \param in Input stream.
 * \param endianness Endianness of the serialized data. Defaults to host's endianness.
 */
template <typename T>
void deserialize_array(T *array, unsigned int size, std::istream &in, Endianness::Type endianness) {
  SerializationTraits<T>::deserialize_array(array, size, in, endianness);
}

/**
 * \brief Serialize the information identifying a type into an output stream.
 *
 * Writes the name (demangled if possible) of the type into the stream.
 * \note Data is serialized with the same endianness as the local host.
 * \param out Output stream.
 */
template <typename T>
void TypeSerializationTraits<T>::serialize_type(std::ostream &out) {
  uint32_t name_length = strlen(Traits<T>::name());
  serialize(name_length, out);
  serialize_array(Traits<T>::name(), name_length, out);
}

/**
 * \brief Deserialize the type information from an input stream.
 *
 * Checks the type name and throws an exception if it doesn't match.
 * \param in Input stream.
 * \param endianness Endianness of the serialized data. Defaults to host's endianness.
 * \exception std::runtime_error Thrown when the serialized type doesn't match \a T.
 */
template <typename T>
void TypeSerializationTraits<T>::check_serialized_type(std::istream &in, Endianness::Type endianness) {

  // Read type name length.
  uint32_t name_length;
  deserialize(name_length, in, endianness);
  if (!in.good())
    throw std::runtime_error("error reading type name length data");

  // Read type name.
  ScopedArray<char> type_name(new char[name_length + 1]);
  deserialize_array(type_name.get(), name_length, in);
  if (!in.good())
    throw std::runtime_error("error reading type name");
  type_name[name_length] = '\0';

  // Check type name.
  // WARNING: The value returned by Traits<T>::name() is the demangled (if supported) version of typeid::name()
  //          and hence is implementation-dependent.
  //          There is a possibility of incompatible files when porting data between different platforms.
  if (strcmp(Traits<T>::name(), type_name.get())) {
    std::string error_msg = "type doesn't match: found ";
    error_msg += type_name.get();
    error_msg += ", expected ";
    error_msg += Traits<T>::name();
    throw std::runtime_error(error_msg);
  }
}

/**
 * \brief Provide if possible the demangled name of the type \a T.
 *
 * The returned value is highly dependent on the platform and the compiler.
 * This method intends to return a full C++ style readable version of the type.
 * It should be extended in the future to include support for extra compilers and platforms.
 */
template <typename T>
const char *NameTraits<T>::name() {
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
 * \brief Swap the endianness of a value.
 * Specialization for fundamental types where is enough to swap bytes in memory.
 *
 * \a value Value to swap endianness.
 */
template <typename T>
void EndiannessTraits<T, true>::swap_endianness(T &value) {
  EndiannessSwapper<T>::run(reinterpret_cast<uint8_t *>(&value));
}

/**
 * \brief Apply endianness swapping to the elements of an array.
 *
 * \param array Array to swap endianness.
 */
template <typename T, const size_t Size>
void swap_endianness(T (&array)[Size]) {
  for (unsigned int i=0; i<Size; ++i)
    EndiannessTraits<T>::swap_endianness(array[i]);
}

} // namespace kche_tree
