CPP=g++
GOPT=gengetopt
CFLAGS=-O3 -Wall
SSE_FLAGS=-msse -DSSE

TEMPLATE_FILES= kche-tree.h kche-tree_sse_24d.h
TEMPLATE_FILES+= kd-tree.h kd-tree.cpp kd-tree_io.cpp
TEMPLATE_FILES+= k-heap.h k-heap.cpp indirect_heap.h indirect_heap.cpp
TEMPLATE_FILES+= k-vector.h k-vector.cpp
TEMPLATE_FILES+= vector.h vector.cpp dataset.h dataset.cpp
TEMPLATE_FILES+= raw-types.h mapreduce.h

MAKEFILE=Makefile

SPEED=speed_kdtree
TEST=test_kdtree
EXAMPLE=example

COMPILE_NOTIFY="  [CC]\t$@"

all: $(SPEED) $(TEST) $(EXAMPLE)

$(SPEED): $(TEMPLATE_FILES) $(SPEED).cpp $(SPEED).ggo $(MAKEFILE) tool_utils.h
	@echo $(COMPILE_NOTIFY)
	@$(GOPT) -a $(SPEED)_args -F $(SPEED)_args -i $(SPEED).ggo
	@$(CPP) $(CFLAGS) $(SPEED).cpp $(SPEED)_args.c -o $@
	@$(CPP) $(CFLAGS) $(SPEED).cpp $(SPEED)_args.c -o $@_sse $(SSE_FLAGS)

$(TEST): $(TEMPLATE_FILES) $(TEST).cpp $(TEST).ggo $(MAKEFILE) tool_utils.h
	@echo $(COMPILE_NOTIFY)
	@$(GOPT) -a $(TEST)_args -F $(TEST)_args -i $(TEST).ggo
	@$(CPP) $(CFLAGS) $(TEST).cpp $(TEST)_args.c -o $@
	@$(CPP) $(CFLAGS) $(TEST).cpp $(TEST)_args.c -o $@_sse $(SSE_FLAGS)

$(EXAMPLE): $(TEMPLATE_FILES) $(EXAMPLE).cpp $(MAKEFILE)
	@echo $(COMPILE_NOTIFY)
	@$(CPP) $(CFLAGS) $(EXAMPLE).cpp -o $@

clean:
	@rm -f $(SPEED) $(SPEED)_args.* $(SPEED)_sse
	@rm -f $(TEST) $(TEST)_args.* $(TEST)_sse
	@rm -f $(EXAMPLE)
