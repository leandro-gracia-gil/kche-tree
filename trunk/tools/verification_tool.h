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
 * \file verification_tool.h
 * \brief Generic verification tool template definition.
 * \author Leandro Graciá Gil
 */

#ifndef _VERIFICATION_TOOL_H_
#define _VERIFICATION_TOOL_H_

// Include argument parsing results from gengetopt.
#include "verification_tool_args.h"

// Tool base class.
#include "tool_base.h"

/**
 * \brief Provide result verification functionality for any given type and metric.
 *
 * K-nearest neighbours and all in range functionalities are tested by comparing the provided
 * results with an exhaustive search. More extra functionalities can be tested by the use
 * of the command line options.
 *
 * \tparam T Type of the element being tested. Requires an additional constructor T(float value) to intialize contents.
 * \tparam D Number of dimensions being tested.
 */
template <typename T, const unsigned int D>
class VerificationTool : public ToolBase<T, D, VerificationOptions> {
public:
  // Tool constructor.
  template <typename RandomEngineType>
  VerificationTool(int argc, char *argv[], RandomEngineType &random_engine);

  // Tool running method.
  template <typename MetricType>
  bool run(const MetricType &metric);

  /// Filename used for testing the serialization of the kd-tree.
  static const char serialization_test_filename[];

private:
  // Command-line option validation.
  bool validate_options() const;
};

// Template implementation.
#include "verification_tool.tpp"

#endif
