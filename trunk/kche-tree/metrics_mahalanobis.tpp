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
 * \file metrics_mahalanobis.tpp
 * \brief Template implementations for the Mahalanobis metric.
 * \author Leandro Graciá Gil
 */

// Include the map-reduce metaprograming templates and traits.
#include "map_reduce.h"
#include "traits.h"

namespace kche_tree {

// Forward declare the specialized functor.
template <typename T, bool isFundamental = IsFundamental<T>::value> struct MahalanobisFullFunctor;
template <typename T, bool isFundamental = IsFundamental<T>::value> struct MahalanobisDiagonalFunctor;

/// Mahalanobis distance functor specialization for fundamental types.
template <typename T>
struct MahalanobisFullFunctor<T, true> {

  /// Type used for accumulator variables.
  typedef typename Traits<T>::AccumulatorType AccumT;

  /// Construct a Mahalanobis distance functor providing the corresponding inverse covariance matrix.
  MahalanobisFullFunctor(const SymmetricMatrix<T> &inverse_covariance) :
    inv_covariance(inverse_covariance) {}

  /// Reference to the inverse covariance matrix used to calculate the distances.
  const SymmetricMatrix<T> &inv_covariance;

  /**
   * \brief Distance operator to calculate the mahalanobis distance from the difference in a given dimension between in 2 arrays.
   *
   * \param acc Accumulator to be updated.
   * \param a First array.
   * \param b Second array.
   * \param i Index of the dimension being calculated.
   * \return A reference to \a acc after being updated. Provided as syntactic sugar.
   */
  AccumT& operator () (AccumT &acc, const T *a, const T *b, unsigned int i) const {
    AccumT aux_acc = Traits<AccumT>::zero();
    for (unsigned int k=0; k<i; ++k)
      aux_acc += (a[k] - b[k]) * inv_covariance(k,i) * 2.0;

    T di = a[i] - b[i];
    aux_acc += di * inv_covariance(i, i);
    return acc += aux_acc * di;
  }
};

/// Mahalanobis distance functor specialization for non-fundamental types.
/// Makes use of the =, -= and *= operators for the \a T type and the += operator for the accumulator type.
template <typename T>
struct MahalanobisFullFunctor<T, false> {

  /// Type used for accumulator variables.
  typedef typename Traits<T>::AccumulatorType AccumT;

  /// Construct a Mahalanobis distance functor providing the corresponding inverse covariance matrix.
  MahalanobisFullFunctor(const SymmetricMatrix<T> &inverse_covariance) :
    inv_covariance(inverse_covariance) {}

  /// Reference to the inverse covariance matrix used to calculate the distances.
  const SymmetricMatrix<T> &inv_covariance;

  /**
   * \brief Distance operator to calculate the mahalanobis distance from the difference in a given dimension between in 2 arrays.
   *
   * \param acc Accumulator to be updated.
   * \param a First array.
   * \param b Second array.
   * \param i Index of the dimension being calculated.
   * \return A reference to \a acc after being updated. Provided as syntactic sugar.
   */
  AccumT& operator () (AccumT &acc, const T *a, const T *b, unsigned int i) const {
    AccumT aux_acc = Traits<AccumT>::zero();
    for (unsigned int k=0; k<i; ++k) {
      T temp = a[k];
      temp -= b[k];
      temp *= inv_covariance(k, i);
      aux_acc += temp;
      aux_acc += temp;
    }

    T di = a[i];
    di -= b[i];
    T temp = di;
    temp *= inv_covariance(i, i);
    aux_acc += temp;
    aux_acc *= di;
    return acc += aux_acc;
  }
};

/// Mahalanobis distance functor simplified for diagonal covariance matrices. Specialization for fundamental types.
template <typename T>
struct MahalanobisDiagonalFunctor<T, true> {

  /// Type used for accumulator variables.
  typedef typename Traits<T>::AccumulatorType AccumT;

  /// Construct a Mahalanobis distance functor providing the corresponding inverse covariance matrix.
  MahalanobisDiagonalFunctor(const SymmetricMatrix<T> &inverse_covariance) :
    inv_covariance(inverse_covariance) {}

  /// Reference to the inverse covariance matrix used to calculate the distances.
  const SymmetricMatrix<T> &inv_covariance;

  /**
   * \brief Distance operator to calculate the mahalanobis distance from the difference in a given dimension between in 2 arrays.
   * Only the diagonal elements from the covariance matrix are used.
   *
   * \param acc Accumulator to be updated.
   * \param a First array.
   * \param b Second array.
   * \param i Index of the dimension being calculated.
   * \return A reference to \a acc after being updated. Provided as syntactic sugar.
   */
  AccumT& operator () (AccumT &acc, const T *a, const T *b, unsigned int i) const {
    T di = a[i] - b[i];
    return acc += di * di * inv_covariance(i, i);
  }
};

/// Mahalanobis distance functor simplified for diagonal covariance matrices. Specialization for non-fundamental types.
/// Makes use of the =, -= and *= operators for the \a T type and the += operator for the accumulator type.
template <typename T>
struct MahalanobisDiagonalFunctor<T, false> {

  /// Type used for accumulator variables.
  typedef typename Traits<T>::AccumulatorType AccumT;

  /// Construct a Mahalanobis distance functor providing the corresponding inverse covariance matrix. The matrix must be diagonal.
  MahalanobisDiagonalFunctor(const SymmetricMatrix<T> &inverse_covariance) :
    inv_covariance(inverse_covariance) {}

  /// Reference to the inverse covariance matrix used to calculate the distances.
  const SymmetricMatrix<T> &inv_covariance;

  /**
   * \brief Distance operator to calculate the mahalanobis distance from the difference in a given dimension between in 2 arrays.
   * Only the diagonal elements from the covariance matrix are used.
   *
   * \param acc Accumulator to be updated.
   * \param a First array.
   * \param b Second array.
   * \param i Index of the dimension being calculated.
   * \return A reference to \a acc after being updated. Provided as syntactic sugar.
   */
  AccumT& operator () (AccumT &acc, const T *a, const T *b, unsigned int i) const {
    T temp = a[i];
    temp -= b[i];
    temp *= temp;
    temp *= inv_covariance(i, i);
    return acc += temp;
  }
};

/// Create a Mahalanobis metric object with the identity as its inverse covariance matrix.
template <typename T, const unsigned int D>
MahalanobisMetric<T, D>::MahalanobisMetric()
    : inv_covariance_(D, true),
      is_diagonal_(true) {}

/**
 * \brief Create a Mahalanobis metric object and calculate its inverse covariance matrix from the provided data set.
 *
 * \param data_set Data set from which the inverse covariance matrix for the metric will be calculated.
 */
template <typename T, const unsigned int D>
MahalanobisMetric<T, D>::MahalanobisMetric(const DataSetType &data_set)
    : inv_covariance_(D, false) {
  set_inverse_covariance(data_set);
}

/**
 * \brief Calculate the inverse covariance matrix of the object from the data of a provided set.
 *
 * To perform this, the covariance matrix of the data in the set will be calculated and stored as a
 * symmetric matrix. Then, this matrix will be inverted using a specialized version of the LDL'
 * decomposition for symmetric matrices. The result is then stored as the inverse covariance matrix.
 *
 * \param data_set Data set from which the inverse covariance matrix for the metric will be calculated.
 */
template <typename T, const unsigned int D>
void MahalanobisMetric<T, D>::set_inverse_covariance(const DataSetType &data_set) {

  // Calculate the set means.
  typedef typename Traits<T>::AccumulatorType AccumT;
  AccumT mean[D];
  for (unsigned int i=0; i<D; ++i)
    mean[i] = Traits<AccumT>::zero();

  unsigned int N = data_set.size();
  for (unsigned int j=0; j<N; ++j)
    for (unsigned int i=0; i<D; ++i)
      mean[i] += data_set[j][i];

  float inv_N = 1.0f / N;
  for (unsigned int i=0; i<D; ++i)
    mean[i] *= inv_N;

  // Calculate the covariance matrix.
  float inv_N1 = 1.0f / (N - 1);
  for (unsigned int j=0; j<D; ++j) {
    for (unsigned int i=0; i<=j; ++i) {

      AccumT acc = Traits<AccumT>::zero();
      for (unsigned int k=0; k<N; ++k) {
        T temp = data_set[k][i];
        temp -= mean[i];
        T temp2 = data_set[k][j];
        temp2 -= mean[j];
        temp *= temp2;
        acc += temp;
      }
      acc *= inv_N1;
      inv_covariance_(j, i) = acc;
    }
  }

  // Invert it to get the inverse covariance matrix.
  inv_covariance_.invert();

  // Assume that the resulting matrix is not a diagonal one.
  is_diagonal_ = false;
}

/**
 * \brief Set explicitly the values of the inverse covariance matrix.
 *
 * The values are assumed to be provided in row order with each row up to the main diagonal (covariance matrices are always symmetric).
 *
 * \param inverse_covariance Row-ordered array of D * (D + 1) / 2 values defining the new inverse covariance matrix.
 */
template <typename T, const unsigned int D>
void MahalanobisMetric<T, D>::set_inverse_covariance(const T *inverse_covariance) {

  // Set the lower triangular values of the matrix (it's symmetric).
  unsigned int k = 0;
  for (unsigned int j=0; j<D; ++j)
    for (unsigned int i=0; i<=j; ++i)
      inv_covariance_(j, i) = inverse_covariance[k++];

  // Assume that the provided matrix is not a diagonal one.
  is_diagonal_ = false;
}

/**
 * \brief Resets the current inverse covariance matrix to a diagonal matrix with the provided values.
 *
 * The values in \a diagonal should be the a set of variances. This method will calculate their
 * multiplicative inverses and store them in the diagonals of the inverse covariance matrix.
 *
 * \param diagonal Array of \c size() diagonal values.
 */
template <typename T, const unsigned int D>
void MahalanobisMetric<T, D>::set_diagonal_covariance(const T *diagonal) {

  for (unsigned int j=0; j<D; ++j) {
    for (unsigned int i=0; i<j; ++i)
      inv_covariance_(j, i) = Traits<T>::zero();
    inv_covariance_(j, j) = diagonal[j];
    Traits<T>::invert(inv_covariance_(j, j));
  }

  is_diagonal_ = true;
}

/**
 * \brief Drop any non-diagonal values from the current inverse covariance matrix.
 * This will enable optimizations that are only available with diagonal covariance matrices.
 */
template <typename T, const unsigned int D>
void MahalanobisMetric<T, D>::force_diagonal_covariance() {
  for (unsigned int j=0; j<D; ++j)
    for (unsigned int i=0; i<j; ++i)
      inv_covariance_(j, i) = Traits<T>::zero();

  is_diagonal_ = true;
}

/**
 * \brief Generic squared Mahalanobis distance operator for two D-dimensional feature vectors.
 * Redefinitions and specializations of this operator are welcome.
 *
 * \param v1 First feature vector.
 * \param v2 Second feature vector.
 * \return Squared Mahalanobis distance between the two vectors.
 */
template <typename T, const unsigned int D>
T MahalanobisMetric<T, D>::operator () (const VectorType &v1, const VectorType &v2) const {

  // Standard squared distance between two D-dimensional vectors.
  typedef typename Traits<T>::AccumulatorType AccumT;
  AccumT acc = Traits<AccumT>::zero();
  if (is_diagonal_)
    MapReduce<T, D>::run(MahalanobisDiagonalFunctor<T>(inv_covariance_), v1.data(), v2.data(), acc);
  else
    MapReduce<T, D>::run(MahalanobisFullFunctor<T>(inv_covariance_), v1.data(), v2.data(), acc);
  return acc;
}

/**
 * \brief Generic squared Mahalanobis distance operator for two D-dimensional feature vectors.
 * Special version with early leaving in case an upper bound value is reached.
 *
 * \param v1 Frist feature vector.
 * \param v2 Second feature vector.
 * \param upper_bound Upper bound for the distance. Will return immediatly if reached.
 * \return Squared Mahalanobis distance between the two vectors or a partial result greater than \a upper.
 */
template <typename T, const unsigned int D>
T MahalanobisMetric<T, D>::operator () (const VectorType &v1, const VectorType &v2, ConstRef_T upper_bound) const {

  // Constant calculated empirically.
  const unsigned int D_acc = (unsigned int) (0.4f * D);

  // Accumulate the first D_acc dimensions without any kind of check.
  typename Traits<T>::AccumulatorType acc = Traits<T>::zero();

  if (is_diagonal_) {
    MahalanobisDiagonalFunctor<T> functor(inv_covariance_);
    MapReduce<T, D_acc>::run(functor, v1.data(), v2.data(), acc);

    // Calculate the remaining dimensions using an upper bound, and checking it every 4 dimensions.
    // The template metaprogramming makes sure this interval is performed without actually checking any index or iterator at runtime.
    // Has been tested to be faster than a loop with the difference being more acute with greater D values.
    BoundedMapReduce<T, D, 4, D_acc>::run(functor, std::greater<T>(), v1.data(), v2.data(), upper_bound, acc);
  } else {
    MahalanobisFullFunctor<T> functor(inv_covariance_);
    MapReduce<T, D_acc>::run(functor, v1.data(), v2.data(), acc);
    BoundedMapReduce<T, D, 4, D_acc>::run(functor, std::greater<T>(), v1.data(), v2.data(), upper_bound, acc);
  }

  return acc;
}

} // namespace kche_tree
