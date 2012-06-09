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
 * \file symmetric_matrix.tpp
 * \brief Template implementation for symmetric matrices.
 * \author Leandro Graciá Gil
 */

// Include traits.
#include "traits.h"

namespace kche_tree {

/// Create a matrix with size 0.
template <typename U>
SymmetricMatrix<U>::SymmetricMatrix() : size_(0) {}

/**
 * \brief Create a matrix of the specified size, optionally initialized to the identity.
 *
 * \param size Size of the matrix to be created.
 * \param initialize_to_identity Will initialize the contents to the identity matrix if \c true, or skip content initialization otherwise.
 */
template <typename U>
SymmetricMatrix<U>::SymmetricMatrix(unsigned int size, bool initialize_to_identity) {
  reset_to_size(size, initialize_to_identity);
}

template <typename U>
SymmetricMatrix<U>::SymmetricMatrix(const SymmetricMatrix &matrix) {
  *this = matrix;
}

template <typename U>
SymmetricMatrix<U>& SymmetricMatrix<U>::operator = (const SymmetricMatrix &matrix) {
  reset_to_size(matrix.size(), false);
  for (unsigned int j=0; j<size_; ++j)
    for (unsigned int i=0; i<=j; ++i)
      m(j, i) = matrix(j, i);

  return *this;
}

/**
 * \brief Resize an existing matrix discarding its contents. May be initialized to the identity.
 *
 * \param size New size of the matrix.
 * \param initialize_to_identity Will initialize the contents to the identity matrix if \c true, or skip content initialization otherwise.
 */
template <typename U>
void SymmetricMatrix<U>::reset_to_size(unsigned int size, bool initialize_to_identity) {

  // Check the empty matrix case.
  size_ = size;
  if (size == 0) {
    column_.reset();
    diagonal_.reset();
    return;
  }

  // Prepare the triangular structure encoding the matrix.
  column_.reset(new ColumnArray[size_]);
  diagonal_.reset(AlignedArray<U>(KCHE_TREE_SSE_RUNTIME_ALIGN(U, size_)));
  initSSEAlignmentGap(diagonal_.get(), size_);

  for (unsigned int i=1; i<size_; ++i) {
    column_[i].reset(AlignedArray<U>(KCHE_TREE_SSE_RUNTIME_ALIGN(U, i)));
    initSSEAlignmentGap(column_[i].get(), i);
  }

  // Initialize contents to zero.
  if (initialize_to_identity) {
    for (unsigned int j=0; j<size_; ++j) {
      for (unsigned int i=0; i<j; ++i)
        m(j, i) = Traits<U>::zero();
      m(j, j) = Traits<U>::one();
    }
  }
}

/**
 * Return a reference to the element located in the specified position of the matrix regardless of the way it's stored.
 * This method is meant to be used internally by the methods of the class.
 *
 * \param row Row of the matrix to access. Behaviour is undefined if greater or equal than \link SymmetricMatrix<U>::size_ size_\endlink.
 * \param column Column of the matrix to access. Behaviour is undefined if greater or equal than \link SymmetricMatrix<U>::size_ size_\endlink.
 */
template <typename U>
U &SymmetricMatrix<U>::m(unsigned int row, unsigned int column) {
  if (row == column)
    return diagonal_[row];
  else if (row > column)
    return column_[row][column];
  else
    return column_[column][row];
}

/**
 * \brief Return a reference to the element located in the specified position of the matrix regardless of the way it's stored.
 *
 * \param row Row of the matrix to access. Behaviour is undefined if greater or equal than \link SymmetricMatrix<U>::size_ size_\endlink.
 * \param column Column of the matrix to access. Behaviour is undefined if greater or equal than \link SymmetricMatrix<U>::size_ size_\endlink.
 */
template <typename U>
U &SymmetricMatrix<U>::operator () (unsigned int row, unsigned int column) {
  return m(row, column);
}

/**
 * \brief Return a const reference to the element located in the specified position of the matrix regardless of the way it's stored.
 *
 * \param row Row of the matrix to access. Behaviour is undefined if greater or equal than \link SymmetricMatrix<U>::size_ size_\endlink.
 * \param column Column of the matrix to access. Behaviour is undefined if greater or equal than \link SymmetricMatrix<U>::size_ size_\endlink.
 */
template <typename U>
const U &SymmetricMatrix<U>::operator () (unsigned int row, unsigned int column) const {
  if (row == column)
    return diagonal_[row];
  else if (row > column)
    return column_[row][column];
  else
    return column_[column][row];
}

/**
 * \brief Invert the matrix using LDL' decomposition optimized for symmetric matrices.
 *
 * This method doesn't require the matrix to be positive definite.
 * \note Matrix contents won't be updated if the matrix is not invertible.
 *
 * \return \c true if successfully inverted, \c false if not invertible.
 */
template <typename U>
bool SymmetricMatrix<U>::invert() {

  // Check empty matrices.
  if (size_ == 0)
    return true;

  // Decompose the matrix into its LDL' decomposition.
  // The LD matrix is the composition of L with D in its diagonal.
  SymmetricMatrix<U> ld(size_, false);
  for (unsigned int j=0; j<size_; ++j) {

    // Calculate the strictly lower triangular values.
    for (unsigned int i=0; i<j; ++i) {

      U acc = Traits<U>::zero();
      for (unsigned int k=0; k<i; ++k) {
        U temp = ld(j, k);
        temp *= ld(i, k);
        temp *= ld(k, k);
        acc += temp;
      }

      ld(j, i) = m(j, i);
      ld(j, i) -= acc;
      ld(j, i) /= ld(i, i);

      // Catch non-invertible matrices.
      if (ld(i, i) == Traits<U>::zero())
        return false;
    }

    // Calculate the diagonal value.
    U acc = Traits<U>::zero();
    for (unsigned int k=0; k<j; ++k) {
      U temp = ld(j, k);
      temp *= ld(j, k);
      temp *= ld(k, k);
      acc += temp;
    }
    ld(j, j) = m(j, j);
    ld(j, j) -= acc;
  }

  // Calculate the inverse of L and D leaving them composed in the same way.
  for (unsigned int j=0; j<size_; ++j) {
    for (unsigned int i=0; i<j; ++i) {
      for (unsigned int k=0; k<i; ++k) {
        U temp = ld(j, i);
        temp *= ld(i, k);
        ld(j, k) -= temp;
      }
      Traits<U>::negate(ld(j, i));
    }

    // Catch non-invertible matrices.
    if (ld(j, j) == Traits<U>::zero())
      return false;

    Traits<U>::invert(ld(j, j));
  }

  // Calculate the inverse matrix from the values of L and D inverted.
  // This stores the result of inv(L)' * inv(D) * inv(L) as the current matrix.
  for (unsigned int j=0; j<size_; ++j) {
    for (unsigned int i=0; i<=j; ++i) {
      U acc = Traits<U>::zero();
      for (unsigned int k=j; k<size_; ++k) {
        U temp = k == j ? Traits<U>::one() : ld(k, j);
        temp *= k == i ? Traits<U>::one() : ld(k, i);
        temp *= ld(k, k);
        acc += temp;
      }
      m(j, i) = acc;
    }
  }

  return true;
}

} // namespace kche_tree
