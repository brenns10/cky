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

# Variable declarations
CC=gcc
FLAGS=
CFLAGS=$(FLAGS) -c -g $(shell if [ -f src/libstephen_conf.h ] ; then echo "-DSMB_CONF" ; fi)
LFLAGS=$(FLAGS)

# Main targets
.PHONY: all clean libstephen_build

all: bin/main

clean:
	rm -rf bin/* obj/* src/libstephen.h src/*.gch && make -C libstephen clean

# Get libstephen.h from the latest version
src/libstephen.h: libstephen/src/libstephen.h
	cp libstephen/src/libstephen.h src/libstephen.h

# Need libstephen.a to build
obj/libstephen.a: libstephen_build
	cp libstephen/bin/libstephen.a obj/libstephen.a

# Must compile libstephen to get static lib.
libstephen_build:
	make -C libstephen lib

# --- Sources
src/gram.c: src/gram.h src/libstephen.h
src/gram.h: src/libstephen.h
src/main.c: src/gram.h
src/fsm.h: src/libstephen.h
src/fsm.c: src/fsm.h src/libstephen.h
src/regex.c: src/regex.h src/fsm.h src/libstephen.h
src/regex.h: src/fsm.h

# --- Objects
OBJECTS = obj/gram.o obj/main.o obj/fsm.o obj/regex.o

obj/gram.o: src/gram.c
	$(CC) $(CFLAGS) src/gram.c -o obj/gram.o

obj/main.o: src/main.c
	$(CC) $(CFLAGS) src/main.c -o obj/main.o

obj/fsm.o: src/fsm.c
	$(CC) $(CFLAGS) src/fsm.c -o obj/fsm.o

obj/regex.o: src/regex.c
	$(CC) $(CFLAGS) src/regex.c -o obj/regex.o

# --- Binaries
bin/main: $(OBJECTS) obj/libstephen.a
	$(CC) $(LFLAGS) $(OBJECTS) obj/libstephen.a -o bin/main
