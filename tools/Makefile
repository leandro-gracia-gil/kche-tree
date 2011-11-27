#=========================================================================#
#   Copyright (C) 2011 by Leandro Graciá Gil                              #
#   leandro.gracia.gil@gmail.com                                          #
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program; if not, write to the                         #
#   Free Software Foundation, Inc.,                                       #
#   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
#=========================================================================#

include ../Makefile.global
include Makefile.settings
include Makefile.tools

PARENT=../
MAKEFILE_DEPS:=$(addprefix $(PARENT), $(MAKEFILE_DEPS)) Makefile Makefile.settings Makefile.tools
KCHE_TREE:=$(addprefix $(PARENT)$(INSTALL_FOLDER)/, $(KCHE_TREE))
INCLUDES=-I$(PARENT)

GENGETOPT_DEPS=gengetopt_found
.PHONY: clean
.PHONY: all

COMPILE_NOTIFY:=$(subst $@,$$@,$(COMPILE_NOTIFY))

define rprefix
$$(subst $$($(1)_prefix),,$(2))
endef

define tool_param
$$(word $(3),$$($(call rprefix,$(1),$(2))))
endef

define tool_remaining_params
$$(wordlist $(3),$$(words $$($(call rprefix,$(1),$(2)))),$$($(call rprefix,$(1),$(2))))
endef

define tool_element_type
$(call tool_param,$(1),$(2),1)
endef

define tool_dimensions
$(call tool_param,$(1),$(2),2)
endef

define tool_metric_file
tool_$(call tool_param,$(1),$(2),3).cpp
endef

define tool_extra_params
$(call tool_remaining_params,$(1),$(2),4)
endef

define tool_settings
$$(patsubst -DCUSTOM_INCLUDE=\"\"$$,,$(call tool_metric_file,$(1),$(2)) -DTYPE=$(call tool_element_type,$(1),$(2)) \
    -DDIMENSIONS=$(call tool_dimensions,$(1),$(2)) $(call tool_extra_params,$(1),$$@))
endef

define build_tool_rule
# Extract tool type settings into variables.
$(1)_file=$$(word 1,$$($(1)_settings))
$(1)_prefix=$$(word 2,$$($(1)_settings))
$(1)_macro=$$(word 3,$$($(1)_settings))
$(1)_gengetopt=$$(word 4,$$($(1)_settings))

# Target with all the tools of the corresponding type to build.
$(1): $$(addprefix $$($(1)_prefix),$$($(1)_tools));

# Target to build the gengetopt files.
$$($(1)_file)_args.%: $$($(1)_file).ggo $$($(1)_file).h $$($(1)_file).tpp;
	@$(GOPT) -a $$($(1)_gengetopt) -F $$($(1)_file)_args -i $$($(1)_file).ggo

# Target to build the actual tools according to each tool's params.
# FIXME: would be good to add tool_metric_file and tool_extra_include as dependencies, but they depend on $$@ and .SECONDEXPANSION doesn't seem to work.
$$(addprefix $$($(1)_prefix),$$($(1)_tools)): $$($(1)_file)_args.[ch] $(GENGETOPT_DEPS) $(MAKEFILE_DEPS) $(KCHE_TREE) $(TOOLS_COMMON);
	@echo $(COMPILE_NOTIFY)
	@$(CPP) $(INCLUDES) $(CFLAGS) $$($(1)_file)_args.c $(call tool_settings,$(1),$$@) -D$$($(1)_macro) -o $$@
endef

all: $(tool_types)

$(foreach tool_type,$(tool_types),$(eval $(call build_tool_rule,$(tool_type))))

$(GENGETOPT_DEPS):
	@if [ -z "`which ${GOPT}`" ]; then \
		echo "Error: gengetopt is required to build the tools, but was not found while looking for '${GOPT}'."; \
		echo "       If the rest of the build was successful you can ignore this error and use the library."; \
		rm -f $@; \
		false; \
	else \
		echo 1 > $@; \
	fi

clean:
	@for tool in $(foreach tool_type,$(tool_types),$(addprefix $($(tool_type)_prefix),$($(tool_type)_tools))); do \
		rm -Rf $$tool $${tool}.dSYM; \
	done
	@for gengetopt_files in $(foreach tool_type,$(tool_types),$($(tool_type)_file)_args); do \
		rm -f $${gengetopt_files}.[ch]; \
	done
	@rm -f kd-tree.test
