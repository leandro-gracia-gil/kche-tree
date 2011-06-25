include ../Makefile.global

PARENT=../
MAKEFILE_DEPS:=$(addprefix $(PARENT), $(MAKEFILE_DEPS)) Makefile
KCHE_TREE:=$(addprefix $(PARENT)$(INSTALL_FOLDER)/, $(KCHE_TREE))
INCLUDES=-I$(PARENT)
.PHONY: clean

# Add any new examples here.
EXAMPLES=knn_simple custom_type

all: $(EXAMPLES)

$(EXAMPLES): %: $(MAKEFILE_DEPS) $(KCHE_TREE) %.cpp
	@echo $(COMPILE_NOTIFY)
	@$(CPP) $(INCLUDES) $(CFLAGS) $@.cpp -o $@

clean:
	@for example in $(EXAMPLES); do \
		rm -Rf $${example} $${example}.dSYM; \
	done
