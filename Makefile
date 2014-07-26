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
# Copyright (c) 2014, Stephen Brennan
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#   * Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#   * Neither the name of Stephen Brennan nor the names of his contributors may
#     be used to endorse or promote products derived from this software without
#     specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL STEPHEN BRENNAN BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#-------------------------------------------------------------------------------

# Compiler Variable Declarations
CC=gcc
FLAGS=
SMB_CONF=$(shell if [ -f src/libstephen_conf.h ] ; then echo "-DSMB_CONF" ; fi)
CFLAGS=$(FLAGS) -c -g --std=c99 $(SMB_CONF)
LFLAGS=$(FLAGS)

# Sources, Headers, and Objects
SOURCES=$(wildcard src/*.c)
HEADERS=$(wildcard src/*.h) src/libstephen.h
OBJECTS=$(patsubst src/%.c,obj/%.o,$(SOURCES)) obj/libstephen.a

# Main targets
.PHONY: all clean libstephen_build

all: bin/main

clean:
	rm -rf bin/* obj/* src/libstephen.h src/*.gch && make -C libstephen clean

# Libstephen compile and header.
src/libstephen.h: libstephen/src/libstephen.h
	cp libstephen/src/libstephen.h src/libstephen.h

obj/libstephen.a: libstephen_build
	cp libstephen/bin/libstephen.a obj/libstephen.a

libstephen_build:
	make -C libstephen lib

# Explicit dependencies in the sources.
src/gram.c: src/gram.h src/libstephen.h
src/gram.h: src/libstephen.h
src/main.c: src/gram.h src/fsm.h src/regex.h
src/fsm.h: src/libstephen.h
src/fsm.c: src/fsm.h src/libstephen.h src/str.h
src/regex.c: src/regex.h src/fsm.h src/str.h src/libstephen.h
src/regex.h: src/libstephen.h src/fsm.h
src/str.c: src/str.h src/fsm.h

# --- Compile Rule
obj/%.o: src/%.c
	$(CC) $(CFLAGS) $< -o $@

# --- Link Rule
bin/main: $(OBJECTS) obj/libstephen.a
	$(CC) $(LFLAGS) $(OBJECTS) -o bin/main
