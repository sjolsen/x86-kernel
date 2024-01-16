BUILDDIR?=../_build

CCSOURCES := $(shell find . -name '*.cc' | xargs realpath --relative-to=.)
CCHEADERS := $(shell find . -name '*.hh' | xargs realpath --relative-to=.)

$(BUILDDIR)/fix_relocations: $(CCSOURCES) $(CCHEADERS)
	g++ -std=c++20 -g -Wall $(CCSOURCES) -o $@
