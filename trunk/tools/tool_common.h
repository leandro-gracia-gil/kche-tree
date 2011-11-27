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
 * \file tool_common.h
 * \brief Define the appropriate tool type depending on the provided macro.
 * \author Leandro Graciá Gil
 */

#ifndef _TOOL_COMMON_H_
#define _TOOL_COMMON_H_

// Include for custom types.
#if defined(CUSTOM_INCLUDE)
#define __ADD_QUOTES(s) __STRINGIFY(s)
#define __STRINGIFY(s) #s
#include __ADD_QUOTES(CUSTOM_INCLUDE)
#endif

// Define the tool parameters.
typedef TYPE ElementType;
static const unsigned int Dimensions = DIMENSIONS;

// Enabled debug assertions in the verification tests.
#if defined(VERIFY)
#define KCHE_TREE_DEBUG
#endif

// Include and use the kche_tree library namespace.
#include "kche-tree/kche-tree.h"
using namespace kche_tree;

// Define the appropriate tool type.
#if defined(BENCHMARK)
#include "benchmark_tool.h"
typedef BenchmarkTool<ElementType, Dimensions> ToolType;
#elif defined(VERIFY)
#include "verification_tool.h"
typedef VerificationTool<ElementType, Dimensions> ToolType;
#else
KCHE_TREE_COMPILE_ASSERT(false, "Error: tool type not defined.");
#endif

#endif
