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
 * \file symmetric_matrix.h
 * \brief Template definition for symmetric matrices.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_SYMMETRIC_MATRIX_H_
#define _KCHE_TREE_SYMMETRIC_MATRIX_H_

// Include smart pointers.
#include "smart_ptr.h"

namespace kche_tree {

/**
 * \brief Symmetric matrix container of the specified size. Stores (n² + n) / 2 elements.
 *
 * \tparam T Data type of the elements in the matrix. Requires the -=, *=, /=, *= float scaling operators and the zero, one, negate and invert traits.
 */
template <typename T>
class SymmetricMatrix {
public:
  /// Type of the elements in the matrix.
  typedef T ElementType;

  // Constructors and initialization methods.
  SymmetricMatrix();
  SymmetricMatrix(unsigned int size, bool initialize_to_identity = true);
  void resetToSize(unsigned int size, bool initialize_to_identity = true);

  // Operators to access the matrix contents.
  T& operator () (unsigned int row, unsigned int column);
  const T& operator () (unsigned int row, unsigned int column) const;

  // Properties of the matrix.
  unsigned int size() const { return size_; } ///< Return the size of the matrix.
  const T *data() const;

  // Matrix operations.
  void invert();

private:
  /// Internal method to access the matrix contents.
  T& m(unsigned int row, unsigned int column);

  unsigned int size_; ///< Size of the matrix.
  ScopedArray<T> base_; ///< Array with the matrix contents flattened in one dimension.
  ScopedArray<T*> m_; ///< Array pointing to the beginning of each row in the matrix.
};

} // namespace kche_tree

// Template implementation.
#include "symmetric_matrix.tpp"

#endif
