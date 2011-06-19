include ../Makefile.global

PARENT='../'
MAKEFILE_DEPS=$(addprefix, $PARENT, $MAKEFILE_DEPS) Makefile
KCHE_TREE=$(addprefix, $PARENT, $KCHE_TREE)
INCLUDES=-I$(addprefix, $PARENT, $INCLUDES)

GENGETOPT_DEPS=gengetopt_found
.PHONY: clean

# Add any common files here.
TOOLS_COMMON=tool_utils.h kche-tree_sse_24d.h

# Add any new tools here.
TOOLS=speed_knn verify

all: $(TOOLS)

$(GENGETOPT_DEPS):
	@if [ -z "`which ${GOPT}`" ]; then \
		echo "Error: gengetopt is required to build the tools, but was not found while looking for '${GOPT}'."; \
		echo "       If the rest of the build was successful you can ignore this error and use the library."; \
		rm -f $@; \
		false; \
	else \
		echo 1 > $@; \
	fi

$(TOOLS): %: $(GENGETOPT_DEPS) $(MAKEFILE_DEPS) $(KCHE_TREE) $(TOOLS_COMMON) %.cpp %.ggo
	@echo $(COMPILE_NOTIFY)
	@$(GOPT) -a $@_args -F $@_args -i $@.ggo
	@$(CPP) $(INCLUDES) $(CFLAGS) $@.cpp $@_args.c -o $@
	@$(CPP) $(INCLUDES) $(CFLAGS) $@.cpp $@_args.c -o $@_sse $(SSE_FLAGS)

clean:
	@for tool in $(TOOLS); do \
	rm -Rf $$tool $${tool}.dSYM $${tool}_args.* $${tool}_sse; done
