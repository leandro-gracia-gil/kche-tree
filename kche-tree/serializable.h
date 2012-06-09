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
 * \file serializable.h
 * \brief Base class for kche-tree serializable objects.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_SERIALIZABLE_H_
#define _KCHE_TREE_SERIALIZABLE_H_

#include <iostream>
#include "endianness.h"

namespace kche_tree {

// Forward declarations.
template <typename T> class Serializable;
template <typename T> std::istream& operator >> (std::istream &in, Serializable<T> &serializable);
template <typename T> std::ostream& operator << (std::ostream &out, const Serializable<T> &serializable);

/**
 * \brief Concept base class for kche-tree serializable objects.
 *
 * Defines methods to serialize and deserialize objects while keeping in mind byte endianness.
 *
 * \tparam Type that should be serializable. Needs to be reimplemented by any further derived classes.
 */
template <typename T>
class Serializable {
protected:
  /// Default constructor.
  Serializable() {}

  /**
   * \brief Serializes the object into a stream using the provided byte endianness.
   * Must be implemented by derived classes.
   *
   * \param out Output stream.
   */
  void serialize(std::ostream &out) const;

  /**
   * \brief Deserializes the contents of an object from a stream, encoded with the provided endianness.
   * Must be also defined as a constructor and implemented by derived classes.
   *
   * \param in Input stream to deserialize from.
   * \param endianness Endianness of the serialized object.
   * \exception std::runtime_error In case of error, with information about the actual problem.
   */
  Serializable(std::istream &in, Endianness::Type endianness);

  /**
   * \brief Swaps the contents of two serializable objects.
   * Must be implemented by derived classes.
   *
   * Used to prevent unsuccessful operations affect the object being deserialized.
   *
   * \param serializable Object to swap contents with.
   */
  void swap(T &serializable);

protected:
  // Type utility and serialization methods. Not required to be implemented by derived classes.
  const char *type_name() const;
  void serialize_type(std::ostream &out) const;
  void check_serialized_type(std::istream &in, Endianness::Type endianness) const;

  // Stream operators. In the case of the >> operator, the argument object will only be modified by successful operations.
  friend std::istream& operator >> <>(std::istream &in, Serializable &serializable);
  friend std::ostream& operator << <>(std::ostream &out, const Serializable &serializable);
};

} // namespace kche_tree

// Template implementation.
#include "serializable.tpp"

#endif
