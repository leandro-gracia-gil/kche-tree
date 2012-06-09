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
 * \file tool_base.tpp
 * \brief Implementation of a base template class for creating tools.
 * \author Leandro Graciá Gil
 */

// C Standard Library and C++ STL includes.
#include <ctime>
#include <iostream>
#include <fstream>

#include "kche-tree/cpp1x.h"

/**
 * \brief Create a new benchmark tool using the options provided in the command line.
 * Data sets are automatically loaded or generated.
 *
 * \tparam RandomEngine Type of the random number generation engine used.
 * \param argc Number of params in command line.
 * \param argv Params of command line.
 * \param random_engine Random number generator used to initialize the contents of the train and test sets if required by the arguments.
 */
template <typename T, unsigned int D, typename L, typename O> template <typename RandomEngine>
ToolBase<T, D, L, O>::ToolBase(int argc, char *argv[], RandomEngine &random_engine)
    : is_ready_(false) {

  // Use gengetopt to parse the command line arguments.
  if (!parse_cmdline(argc, argv))
    return;

  // Verify the provided options_->
  if (!validate_options())
    return;

  // Initialize the data sets.
  if (!initialize_data(random_engine))
    return;

  // Tool is ready.
  is_ready_ = true;
}

/// Release any command line parsing information.
template <typename T, unsigned int D, typename L, typename O>
ToolBase<T, D, L, O>::~ToolBase() {
  if (options_)
    cmdline_parser_free(options_.get());
}

/**
 * \brief Parse command line arguments into the \link ToolBase::options_ options\endlink structure (uses gengetopt).
 *
 * \param argc Number of params in command line.
 * \param argv Params of command line.
 * \return \c false if no arguments are provided, on parse error or if help or the version are required. \c true otherwise, indicating further actions to be done.
*/
template <typename T, unsigned int D, typename L, typename O>
bool ToolBase<T, D, L, O>::parse_cmdline(int argc, char *argv[]) {

  // Print the help if no arguments are provided.
  if (argc == 1) {
    cmdline_parser_print_help();
    return false;
  }

  // Parse the command line arguments.
  options_.reset(new CommandLineOptions);
  if (cmdline_parser(argc, argv, options_.get()) != 0) {
    options_.reset();
    std::cerr << "Run " << argv[0] << " --help or -h to see the list of available options_->" << std::endl;
    return false;
  }

  // Print the help if required.
  if (options_->help_given) {
    cmdline_parser_print_help();
    return false;
  }

  // Print the version if required.
  if (options_->version_given) {
    cmdline_parser_print_version();
    return false;
  }

  return true;
}

/**
 * \brief Validate the values from the provided options.
 * Should be explicitely called if redefined by derived classes.
 *
 * \return \c true if valid, \c false otherwise.
 */
template <typename T, unsigned int D, typename L, typename O>
bool ToolBase<T, D, L, O>::validate_options() const {

  if (options_->train_random_given && options_->train_random_arg <= 0) {
    std::cerr << "Invalid random train set size." << std::endl;
    return false;
  }

  if (options_->test_random_given && options_->test_random_arg <= 0) {
    std::cerr << "Invalid random test set size." << std::endl;
    return false;
  }

  if (options_->test_from_train_arg < 0.0f || options_->test_from_train_arg > 100.0f) {
    std::cerr << "Invalid test-from-train value. Should be between 0 and 100 (%)." << std::endl;
    return false;
  }

  return true;
}

/**
 * \brief Initialize data according to the provided options, either loading or randomly-generating it.
 *
 * \tparam RandomEngine Type of the random number generation engine used.
 * \param random_engine Random number generator used to initialize the contents of the train and test sets if required by the arguments.
 * \return \c true if successful, \c false otherwise.
 */
template <typename T, unsigned int D, typename L, typename O> template <typename RandomEngine>
bool ToolBase<T, D, L, O>::initialize_data(RandomEngine &random_engine) {

  // Initialize the random seed.
  if (options_->random_seed_given)
    random_engine.seed(options_->random_seed_arg);
  else
    random_engine.seed(time(NULL));

  // Get the train set data using the given command line options.
  if (!prepare_train_set(random_engine))
    return false;

  // Get the test set data using the given command line options.
  if (!prepare_test_set(random_engine))
    return false;

  return true;
}

/**
 * \brief Provide the data of the train set using the given options.
 *
 * Data is provided by either reading from a file or randomly generated, using the options passed to the command line.
 *
 * \tparam RandomEngine Type of the random number generation engine used.
 * \param random_engine Random number generator engine used to generate the dataset contents.
 * \return \c true if successful, \c false otherwise.
 */
template <typename T, unsigned int D, typename L, typename O> template <typename RandomEngine>
bool ToolBase<T, D, L, O>::prepare_train_set(RandomEngine &random_engine) {

  using namespace std;
  if (options_->train_file_given) {
    ifstream input(options_->train_file_arg, ios::in | ios::binary);
    if (!input.good()) {
      cerr << "Error opening file '" << options_->train_file_arg << "' for reading." << endl;
      return false;
    }
    input >> train_set_;

  } else if (options_->train_random_given) {
    // Create a random value generator.
    typedef typename kche_tree::Traits<T>::UniformDistribution ValueDistribution;
    typedef typename kche_tree::Traits<T>::RandomDistributionElement DistributionElement;
    ValueDistribution value_distribution(DistributionElement(options_->random_range_min_arg), DistributionElement(options_->random_range_max_arg));
    RandomGenerator<RandomEngine, ValueDistribution> value_generator(random_engine, value_distribution);

    train_set_.reset_to_size(options_->train_random_arg);
    train_set_.set_random_values(value_generator);

    if (options_->train_save_random_given) {
      ofstream output(options_->train_save_random_arg, ios::out | ios::binary);
      if (!output.good()) {
        cerr << "Error opening file '" << options_->train_save_random_arg << "' for writing." << endl;
        return false;
      }
      output << train_set_;
    }
  } else {
    // Should never reach this point. If we do gengetopt is failing.
    KCHE_TREE_NOT_REACHED();
  }

  return true;
}

/**
 * \brief Provide the data of the test set using the given options.
 *
 * Data is provided by either reading from a file or randomly generated, using the options passed to the command line.
 * The test set can have elements copied from the train set. The probability of these copies is provided by the options.
 *
 * \tparam RandomEngine Type of the random number generation engine used.
 * \param random_engine Random number generator engine used to generate the dataset contents.
 * \return \c true if successful, \c false otherwise.
 */
template <typename T, unsigned int D, typename L, typename O> template <typename RandomEngine>
bool ToolBase<T, D, L, O>::prepare_test_set(RandomEngine &random_engine) {

  using namespace std;
  if (options_->test_file_given) {
    ifstream input(options_->test_file_arg, ios::in | ios::binary);
    if (!input.good()) {
      cerr << "Error opening file '" << options_->test_file_arg << "' for reading." << endl;
      return false;
    }
    input >> test_set_;

  } else if (options_->test_random_given) {
    // Create random generators for elements, probabilities and indices.
    typedef typename kche_tree::Traits<T>::UniformDistribution ValueDistribution;
    typedef typename kche_tree::Traits<float>::UniformDistribution ProbabilityDistribution;
    typedef typename kche_tree::Traits<unsigned int>::UniformDistribution IndexDistribution;

    typedef typename kche_tree::Traits<T>::RandomDistributionElement DistributionElement;
    ValueDistribution value_distribution(DistributionElement(options_->random_range_min_arg), DistributionElement(options_->random_range_max_arg));
    ProbabilityDistribution probability_distribution(0.0f, 100.0f);
    IndexDistribution index_distribution(0, train_set_.size() - 1);

    RandomGenerator<RandomEngine, ValueDistribution> value_generator(random_engine, value_distribution);
    RandomGenerator<RandomEngine, ProbabilityDistribution> probability_generator(random_engine, probability_distribution);
    RandomGenerator<RandomEngine, IndexDistribution> index_generator(random_engine, index_distribution);

    float p = options_->test_from_train_arg;
    unsigned int size = options_->test_random_arg;
    test_set_.reset_to_size(size);

    for (unsigned int i=0; i<size; ++i) {
      if (probability_generator() < p) {
        unsigned int reference_index = index_generator();
        kche_tree::Traits<T>::copy_array(&test_set_[i][0], &train_set_[reference_index][0], D);
      } else {
        for (unsigned int d=0; d<D; ++d)
          test_set_[i][d] = kche_tree::Traits<T>::random(value_generator);
      }
    }

    if (options_->test_save_random_given) {
      ofstream output(options_->test_save_random_arg, ios::out | ios::binary);
      if (!output.good()) {
        cerr << "Error opening file '" << options_->test_save_random_arg << "' for writing." << endl;
        return false;
      }
      output << test_set_;
    }
  } else {
    // Should never reach this point. If we do gengetopt is failing.
    KCHE_TREE_NOT_REACHED();
  }

  return true;
}
