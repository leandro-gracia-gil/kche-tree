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
 * \file tool_utils.h
 * \brief Auxiliary functions used by the library tools.
 * \author Leandro Graciá Gil
 */

// C Standard Library and C++ STL includes.
#include <cassert>
#include <fstream>

// Library includes.
#include "kche-tree/dataset.h"
#include "kche-tree/raw-types.h"

/**
 * \brief Generate a random data set.
 *
 * Generate a random dataset of the specified size and numeric range.
 *
 * \param size Number of vectors in the dataset.
 * \param range Numeric range of the elements in the vectors. Will go from 0 to \a range.
 * \param data Generated data set. Will discard any existing contents.
 */
template <typename T, const unsigned int D>
void generate_random_dataset(unsigned int size, T range, kche_tree::DataSet<T, D>& random_set) {
  random_set.reset_to_size(size);
  for (unsigned int i=0; i<size; ++i)
    for (unsigned int d=0; d<D; ++d)
      random_set[i][d] = static_cast<T>(rand() / (float) RAND_MAX) * range;
}

/**
 * \brief Generate a random data set using elements from another.
 *
 * Generate a random dataset of the specified size and numeric range using elements from another set with a given probability.
 *
 * \param size Number of vectors in the dataset.
 * \param range Numeric range of the elements in the vectors. Will go from 0 to \a range.
 * \param reference_set Reference dataset from where some elements may be taken.
 * \param p Probability (from 0 to 1) of copying an element from the reference set instead of generating a random one.
 * \param data Generated data set. Will discard any existing contents.
 */
template <typename T, const unsigned int D>
void generate_random_dataset_from_existing(unsigned int size, T range, const kche_tree::DataSet<T, D> &reference_set, float p, kche_tree::DataSet<T, D> &random_set) {
  random_set.reset_to_size(size);
  for (unsigned int i=0; i<size; ++i) {
    if (rand() / static_cast<float>(RAND_MAX) < p) {
      unsigned int reference_index = rand() % reference_set.size();
      kche_tree::copy_array(&random_set[i][0], &reference_set[reference_index][0], D);
    } else {
      for (unsigned int d=0; d<D; ++d)
        random_set[i][d] = static_cast<T>(rand() / static_cast<float>(RAND_MAX)) * range;
    }
  }
}

/**
 * \brief Provide the data of the train set using the given options.
 *
 * Provide the data of the train set, either read from a file or randomly generated, using the options passed to the command line.
 *
 * \param options Command-line options structure provided by gengetopt.
 * \param train_set Training set to be filled with the required data.
 * \return \c true if successful, \c false otherwise.
 */
template <typename Options, typename T, const unsigned int D>
bool get_train_set_data(const Options &options, kche_tree::DataSet<T, D> &train_set) {

  using namespace std;
  if (options.train_file_given) {
    ifstream input(options.train_file_arg, ios::in | ios::binary);
    if (!input.good()) {
      cerr << "Error opening file '" << options.train_file_arg << "' for reading." << endl;
      return false;
    }
    input >> train_set;

  } else if (options.train_random_given) {
    generate_random_dataset(options.train_random_arg, options.random_range_arg, train_set);

    if (options.train_save_random_given) {
      ofstream output(options.train_save_random_arg, ios::out | ios::binary);
      if (!output.good()) {
        cerr << "Error opening file '" << options.train_save_random_arg << "' for writing." << endl;
        return false;
      }
      output << train_set;
    }
  } else {
    // Should never reach this point. If we do gengetopt is failing.
    assert(0);
  }

  return true;
}

/**
 * \brief Provide the data of the test set using the given options.
 *
 * Provide the data of the test set, either read from a file or randomly generated, using the options passed to the command line.
 *
 * \param options Command-line options structure provided by gengetopt.
 * \param train_set Train set that can be used as source of some of the test vectors.
 * \param test_set Test set to be filled with the required data.
 * \return \c true if successful, \c false otherwise.
 */
template <typename Options, typename T, const unsigned int D>
bool get_test_set_data(const Options &options, const kche_tree::DataSet<T, D> &train_set, kche_tree::DataSet<T, D> &test_set) {

  using namespace std;
  if (options.test_file_given) {
    ifstream input(options.test_file_arg, ios::in | ios::binary);
    if (!input.good()) {
      cerr << "Error opening file '" << options.test_file_arg << "' for reading." << endl;
      return false;
    }
    input >> test_set;

  } else if (options.test_random_given) {
    generate_random_dataset_from_existing(options.test_random_arg, options.random_range_arg, train_set, options.test_from_train_arg * 0.01f, test_set);

    if (options.test_save_random_given) {
      ofstream output(options.test_save_random_arg, ios::out | ios::binary);
      if (!output.good()) {
        cerr << "Error opening file '" << options.test_save_random_arg << "' for writing." << endl;
        return false;
      }
      output << test_set;
    }
  } else {
    // Should never reach this point. If we do gengetopt is failing.
    assert(0);
  }

  return true;
}
