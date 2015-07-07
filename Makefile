#-------------------------------------------------------------------------------
#
# File:         Makefile
#
# Author:       Stephen Brennan
#
# Date Created: Tuesday, 13 May 2014
#
# Description:  Makefile for CKY parser program.
#
# Copyright (c) 2014-2015, Stephen Brennan.  Released under the Revised BSD
# License.  See the LICENSE.txt file for details.
#
#-------------------------------------------------------------------------------

# Compiler Variable Declarations
CC=gcc
FLAGS=-Wall -pedantic
INC=-Ilibstephen/inc/ -Isrc/
CFLAGS=$(FLAGS) -c -g --std=c99 $(SMB_CONF) $(INC)
LFLAGS=$(FLAGS)
DIR_GUARD=@mkdir -p $(@D)

# Build configurations.
CFG=release
ifeq ($(CFG),debug)
FLAGS += -g -DDEBUG -DSMB_DEBUG
endif
ifeq ($(CFG),coverage)
CFLAGS += -fprofile-arcs -ftest-coverage
LFLAGS += -fprofile-arcs -lgcov
endif
ifneq ($(CFG),debug)
ifneq ($(CFG),release)
ifneq ($(CFG),coverage)
	@echo "Invalid configuration "$(CFG)" specified."
	@echo "You must specify a configuration when running make, e.g."
	@echo "  make CFG=debug"
	@echo "Choices are 'release', 'debug', and 'coverage'."
	@exit 1
endif
endif
endif

# Sources and Objects
SOURCES=$(shell find src/ -type f -name "*.c")
OBJECTS=$(patsubst src/%.c,obj/$(CFG)/%.o,$(SOURCES)) obj/$(CFG)/libstephen.a
DEPS=$(patsubst src/%.c,deps/%.d,$(SOURCES))

# Main targets
.PHONY: all clean clean_all clean_docs clean_cov libstephen_build docs gcov

all: bin/$(CFG)/main

test: bin/$(CFG)/main
	valgrind bin/$(CFG)/main -t

gcov:
	lcov -c -d obj/coverage -b src -o coverage.info
	genhtml coverage.info -o cov/
	lcov -r coverage.info "`pwd`/src/test/*" -o coverage.info
	rm coverage.info

clean:
	rm -rf bin/$(CFG)/* obj/$(CFG)/* src/libstephen.h src/*.gch
	make -C libstephen CFG=$(CFG) clean

clean_all: clean clean_docs clean_cov
	rm -rf bin/* obj/*
	make -C libstephen CFG=$(CFG) clean_all

clean_docs:
	rm -rf doc/*

clean_cov:
	rm -rf cov/*

docs: $(SOURCES)
	doxygen

# Libstephen compile and header.
obj/$(CFG)/libstephen.a: libstephen_build
	$(DIR_GUARD)
	cp libstephen/bin/$(CFG)/libstephen.a obj/$(CFG)/libstephen.a

libstephen_build:
	make -C libstephen CFG=$(CFG) lib

# --- Compile Rule
obj/$(CFG)/%.o: src/%.c
	$(DIR_GUARD)
	$(CC) $(CFLAGS) $< -o $@

# --- Link Rule
bin/$(CFG)/main: $(OBJECTS) obj/$(CFG)/libstephen.a
	$(DIR_GUARD)
	$(CC) $(LFLAGS) $(OBJECTS) -o bin/$(CFG)/main

# --- Dependency Rule
deps/%.d: src/%.c
	$(DIR_GUARD)
	$(CC) $(CFLAGS) -MM $< | sed -e 's/~\(.*\)\.o:/\1.d \1.o:/' > $@

ifneq "$(MAKECMDGOALS)" "clean_all"
-include $(DEPS)
endif
