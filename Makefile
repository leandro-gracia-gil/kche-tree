CPP=g++
GOPT=gengetopt
CFLAGS=-O3 -Wall
SSE_FLAGS=-msse -DSSE

TEMPLATE_FILES= kche-tree.h kche-tree_sse_24d.h
TEMPLATE_FILES+= kd-tree.h kd-tree.tpp kd-tree_io.tpp
TEMPLATE_FILES+= k-heap.h k-heap.tpp indirect_heap.h indirect_heap.tpp
TEMPLATE_FILES+= k-vector.h k-vector.tpp
TEMPLATE_FILES+= vector.h vector.tpp dataset.h dataset.tpp
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
	@rm -Rf $(SPEED) $(SPEED).dSYM $(SPEED)_args.* $(SPEED)_sse
	@rm -Rf $(TEST) $(TEST).dSYM $(TEST)_args.* $(TEST)_sse
	@rm -Rf $(EXAMPLE) $(EXAMPLE).dSYM
