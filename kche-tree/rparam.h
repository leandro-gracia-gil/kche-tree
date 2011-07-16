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
 * \file rparam.h
 * \brief Define type optimization templates for const arguments.
 * \author Leandro Graciá Gil
 */

#ifndef _KCHE_TREE_RPARAM_H_
#define _KCHE_TREE_RPARAM_H_

namespace kche_tree {

/**
 * \brief Type branching template.
 * Defines its type Result depending on the value of \a Cond.
 *
 * \tparam Condition to evaluate on compile time.
 * \tparam A Type of \a Result if \a Cond is \c true.
 * \tparam B Type of \a Result if \a Cond is \c false.
 */
template <bool Cond, typename A, typename B>
struct TypeBranch;

/// Type branching template specialization for the \c true case.
template <typename A, typename B>
struct TypeBranch<true, A, B> {
  /// Same as the template parameter \a A.
  typedef A Result;
};

/// Type branching template specialization for the \c false case.
template <typename A, typename B>
struct TypeBranch<false, A, B> {
  /// Same as the template parameter \a B.
  typedef B Result;
};

/**
 * \brief Provides the type to use for const reference parameters in a smart way.
 *
 * The parameter will be passed by value instead as a const reference if
 * the type is smaller and the properties of the type allows it.
 *
 * \tparam T Type of the argument without any qualifications.
 */
template <typename T>
struct RParam {
  typedef typename TypeBranch<
#ifdef KCHE_TREE_DISABLE_CPP0X
      std::tr1::is_pod<T>::value &&
      std::tr1::has_trivial_constructor<T>::value &&
      std::tr1::has_trivial_copy<T>::value &&
      std::tr1::has_trivial_destructor<T>::value &&
#else
      std::is_pod<T>::value &&
      std::has_trivial_default_constructor<T>::value &&
      std::has_trivial_copy_constructor<T>::value &&
      std::has_trivial_destructor<T>::value &&
#endif
      sizeof(T) <= sizeof(const T&), T, const T&>::Result Type;
};

} // namespace kche_tree

#endif
