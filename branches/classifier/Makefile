CPP=g++
CFLAGS=-O3 -Wall

SSE_FLAGS=-msse -DSSE
FILE_FLAGS=-DFROM_FILE

TEMPLATE_FILES= kd-tree.h kd-tree.cpp kd-tree_io.cpp kd-tree_sse_24d.h
TEMPLATE_FILES+= k-heap.h k-heap.cpp indirect_heap.h indirect_heap.cpp
TEMPLATE_FILES+= k-vector.h k-vector.cpp
TEMPLATE_FILES+= feature_vector.h feature_vector.cpp

SPEED=speed_kdtree
TEST=test_kdtree
EXAMPLE=example

COMPILE_NOTIFY="  [CC]\t$@"

all: $(SPEED) $(TEST) $(EXAMPLE)

$(SPEED): $(TEMPLATE_FILES) $(SPEED).cpp
	@echo $(COMPILE_NOTIFY)
	@$(CPP) $(CFLAGS) $(SPEED).cpp -o $@
	@$(CPP) $(CFLAGS) $(SPEED).cpp -o $@_file $(FILE_FLAGS)
	@$(CPP) $(CFLAGS) $(SPEED).cpp -o $@_sse $(SSE_FLAGS)
	@$(CPP) $(CFLAGS) $(SPEED).cpp -o $@_file_sse $(SSE_FLAGS) $(FILE_FLAGS)

$(TEST): $(TEMPLATE_FILES) $(TEST).cpp
	@echo $(COMPILE_NOTIFY)
	@$(CPP) $(CFLAGS) $(TEST).cpp -o $@
	@$(CPP) $(CFLAGS) $(TEST).cpp -o $@_file $(FILE_FLAGS)
	@$(CPP) $(CFLAGS) $(TEST).cpp -o $@_sse $(SSE_FLAGS)
	@$(CPP) $(CFLAGS) $(TEST).cpp -o $@_file_sse $(SSE_FLAGS) $(FILE_FLAGS)

$(EXAMPLE): $(TEMPLATE_FILES) $(EXAMPLE).cpp
	@echo $(COMPILE_NOTIFY)
	@$(CPP) $(CFLAGS) $(EXAMPLE).cpp -o $@

clean:
	@rm -f $(SPEED) $(SPEED)_file $(SPEED)_sse $(SPEED)_file_sse
	@rm -f $(TEST) $(TEST)_file $(TEST)_sse $(TEST)_file_sse
	@rm -f $(EXAMPLE)

