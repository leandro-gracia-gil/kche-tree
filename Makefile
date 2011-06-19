include Makefile.global

# Add any new subdirs here.
SUBDIRS=examples tools

.PHONY: subdirs $(SUBDIRS)
.PHONY: clean

all: $(SUBDIRS)
subdirs: $(SUBDIRS)

$(SUBDIRS):
	@echo "Building $@..."
	@$(MAKE) -sC $@

clean:
	@for i in $(SUBDIRS); do \
	echo "Cleaning $$i..."; \
	(cd $$i; $(MAKE) clean); done
