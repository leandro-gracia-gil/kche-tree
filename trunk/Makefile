include Makefile.global

# Add any new subdirs here.
SUBDIRS=examples tools

.PHONY: subdirs $(SUBDIRS)
.PHONY: clean
.PHONY: install

all: $(SUBDIRS)
subdirs: $(SUBDIRS)

$(SUBDIRS):
	@echo "Building $@..."
	@$(MAKE) -sC $@

install:
	@mkdir -p $(PREFIX)/$(INSTALL_FOLDER)
	@$(INSTALL) -m 644 $(INSTALL_FOLDER)/* $(PREFIX)/$(INSTALL_FOLDER) && \
		echo "Kche-tree successfully installed."

uninstall:
	@if [ -d $(PREFIX)/$(INSTALL_FOLDER) ]; then \
		rm -Rf $(PREFIX)/$(INSTALL_FOLDER) && \
			echo "Kche-tree successfully uninstalled."; \
	else \
		echo "Can't find Kche-tree installed in the system."; \
	fi

clean:
	@for i in $(SUBDIRS); do \
		echo "Cleaning $$i..."; \
		(cd $$i; $(MAKE) clean); \
	done
