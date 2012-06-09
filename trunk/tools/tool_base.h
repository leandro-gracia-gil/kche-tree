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
 * \file tool_base.h
 * \brief Definition of a base template class for creating tools.
 * \author Leandro Graciá Gil
 */

#ifndef _TOOL_BASE_H_
#define _TOOL_BASE_H_

// Include the kche-tree library.
#include "kche-tree/kche-tree.h"

/**
 * \brief Provide tool initialization and execution for specific tool types.
 *
 * \tparam ElementType Type of the element being tested.
 * \tparam NumDimensions Number of dimensions being tested.
 * \tparam LabelType Type of the labels in the data sets.
 * \tparam CommandLineOptionsType Type of the gengetopt structure providing parsed command-line options.
 */
template <typename ElementType, const unsigned int NumDimensions, typename LabelType, typename CommandLineOptionsType>
class ToolBase {
public:
  /// Type of the elements in the test.
  typedef ElementType Element;

  /// Number of dimensions in the test.
  static unsigned const int Dimensions = NumDimensions;

  /// Type of the labels associated to the feature vectors. Equals to void if no labels are used.
  typedef LabelType Label;

  /// Type of the command line options structure.
  typedef CommandLineOptionsType CommandLineOptions;

  /// Corresponding kd-tree type.
  typedef kche_tree::KDTree<Element, Dimensions, Label> KDTree;

  /// Corresponding data set type.
  typedef typename KDTree::DataSet DataSet;

  // Constructor and destructor.
  template <typename RandomEngineType>
  ToolBase(int argc, char *argv[], RandomEngineType &random_engine);
  ~ToolBase();

  // Tool properties.
  bool is_ready() const { return is_ready_; } ///< Check if the tool is ready to be run. May not be the case if the options or the data sets failed.
  const CommandLineOptions &options() const { return options_; } ///< Return the current set of options provided by the command line arguments.
  const DataSet &train_set() const { return train_set_; } ///< Return the train set used by the tool.
  const DataSet &test_set() const { return test_set_; } ///< Return the test set used by the tool.

protected:
  // Initialization, parsing and validation.
  template <typename RandomEngineType>
  bool initialize_data(RandomEngineType &random_engine);
  bool parse_cmdline(int argc, char *argv[]);
  bool validate_options() const;

  // Data set preparation.
  template <typename RandomEngineType>
  bool prepare_train_set(RandomEngineType &engine);

  template <typename RandomEngineType>
  bool prepare_test_set(RandomEngineType &engine);

  ScopedPtr<CommandLineOptions> options_; ///< Gengetopt structure containing the parsed command line arguments.
  bool is_ready_; ///< Flag indicating if the tool is ready to be run.

  DataSet train_set_; ///< Train set used by the tool.
  DataSet test_set_; ///< Test set used by the tool.
};

// Template implementation.
#include "tool_base.tpp"

#endif
