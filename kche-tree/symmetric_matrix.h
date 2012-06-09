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

// Include scoped arrays.
#include "scoped_ptr.h"

namespace kche_tree {

/**
 * \brief Symmetric matrix container of the specified size. Stores (n² + n) / 2 elements.
 *
 * For memory alignment and cache reasons, data is stored in a particular way.
 * The upper triangular part of the matrix is stored column-wise, but the elements of the
 * main diagonal are stored separately. So, each column stores the values above the diagonal.
 *
 * Additionally, if SSE is enabled the start of every column vector and the diagonal are ensured to be 16-byte aligned.
 *
 * For example, listed by the contents of each column vector and using the (row, column) notation
 * the following would be an example of storage for a symmetric 5x5 matrix:
 *
 * 0:
 * 1: (0,1)
 * 2: (0,2), (1,2)
 * 3: (0,3), (1,3), (2,3)
 * 4: (0,4), (2,4), (2,4), (3,4)
 *
 * diagonal: (0,0), (1,1), (2,2), (3,3), (4,4)
 *
 * \tparam U Data type of the elements in the matrix. Requires the -=, *=, /=, *= float scaling operators and the zero, one, negate and invert traits.
 * \note The type \a U corresponds to the Distance type for the elements in the kd-tree, not to the elements themselves.
 */
template <typename U>
class SymmetricMatrix {
public:
  /// Type of the elements in the matrix.
  typedef U Element;

  // Constructors and initialization methods.
  SymmetricMatrix();
  SymmetricMatrix(unsigned int size, bool initialize_to_identity = true);
  void reset_to_size(unsigned int size, bool initialize_to_identity = true);

  // Copy constructor and assignment operator.
  SymmetricMatrix(const SymmetricMatrix &matrix);
  SymmetricMatrix& operator = (const SymmetricMatrix &matrix);

  // Operators to access the matrix contents.
  U & operator () (unsigned int row, unsigned int column);
  const U & operator () (unsigned int row, unsigned int column) const;

  // Properties of the matrix.
  unsigned int size() const { return size_; } ///< Return the size of the matrix.
  const U *column(unsigned int column) const { return column_[column].get(); } ///< Return a pointer to the column elements before the diagonal.
  const U *diagonal() const { return diagonal_.get(); } ///< Return a pointer to the diagonal elements.

  // Matrix operations.
  bool invert();

private:
  /// Internal method to access the matrix contents.
  U& m(unsigned int row, unsigned int column);

  typedef ScopedAlignedArray<U> ColumnArray;

  unsigned int size_; ///< Size of the matrix.
  ScopedArray<ColumnArray> column_; ///< Array pointing to the beginning of each column in the matrix.
  ColumnArray diagonal_;
};

} // namespace kche_tree

// Template implementation.
#include "symmetric_matrix.tpp"

#endif
