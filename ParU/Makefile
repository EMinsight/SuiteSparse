#-------------------------------------------------------------------------------
# SuiteSparse/ParU/Makefile
#-------------------------------------------------------------------------------

# ParU, Copyright (c) 2022-2024, Mohsen Aznaveh and Timothy A. Davis,
# All Rights Reserved.
# SPDX-License-Identifier: GPL-3.0-or-later

#-------------------------------------------------------------------------------

# A simple Makefile for ParU, which relies on cmake to do the
# actual build.  All the work is done in cmake so this Makefile is just for
# convenience.

# To compile with an alternate compiler:
#
#       make CC=gcc CXX=g++
#
# To compile/install for system-wide usage:
#
#       make
#       sudo make install
#
# To compile/install for local usage (SuiteSparse/lib and SuiteSparse/include):
#
#       make local
#       make install
#
# To clean up the files:
#
#       make clean

JOBS ?= 8

default: library

library:
	( cd build && cmake $(CMAKE_OPTIONS) .. && cmake --build . --config Release -j${JOBS} )

# install only in SuiteSparse/lib and SuiteSparse/include
local:
	( cd build && cmake $(CMAKE_OPTIONS) -USUITESPARSE_PKGFILEDIR -DSUITESPARSE_LOCAL_INSTALL=1 .. && cmake --build . --config Release -j${JOBS} )

# install only in /usr/local (default)
global:
	( cd build && cmake $(CMAKE_OPTIONS) -USUITESPARSE_PKGFILEDIR -DSUITESPARSE_LOCAL_INSTALL=0 .. && cmake --build . --config Release -j${JOBS} )

debug:
	( cd build && cmake $(CMAKE_OPTIONS) -DCMAKE_BUILD_TYPE=Debug .. && cmake --build . --config Release -j${JOBS} )

all: library

demos: library
	( cd build && cmake $(CMAKE_OPTIONS) -DSUITESPARSE_DEMOS=1 .. && cmake --build . --config Release -j${JOBS} )
	( cd build && ./paru_simplec$(EXEEXT) < ../Matrix/b1_ss.mtx )
	( cd build && ./paru_simple$(EXEEXT) < ../Matrix/west0067.mtx )
	( cd build && [ -f paru_democ$(EXEEXT) ] && ./paru_democ$(EXEEXT) < ../Matrix/west0067.mtx || true )
	( cd build && [ -f paru_demo$(EXEEXT) ] && ./paru_demo$(EXEEXT) < ../Matrix/xenon1.mtx || true )

bench: library
	( cd build && [ -f paru_benchmark$(EXEEXT) ] && ./paru_benchmark$(EXEEXT) < ../Matrix/xenon1.mtx || true )

V = valgrind --leak-check=full --show-leak-kinds=all

vdemos: library
	( cd build && cmake $(CMAKE_OPTIONS) -DSUITESPARSE_DEMOS=1 .. && cmake --build . --config Release -j${JOBS} )
	( cd build && $(V) ./paru_simplec$(EXEEXT) < ../Matrix/b1_ss.mtx )
	( cd build && $(V) ./paru_simple$(EXEEXT) < ../Matrix/west0067.mtx )
	( cd build && [ -f paru_democ$(EXEEXT) ] && $(V) ./paru_democ$(EXEEXT) < ../Matrix/west0067.mtx || true )
	( cd build && [ -f paru_demo$(EXEEXT) ] && $(V) ./paru_demo$(EXEEXT) < ../Matrix/xenon1.mtx || true )

w: library
	( cd build && cmake $(CMAKE_OPTIONS) -DSUITESPARSE_DEMOS=1 .. && cmake --build . --config Release -j${JOBS} )
	( cd build && $(V) ./paru_demo$(EXEEXT) < ../Matrix/west0479.mtx )

cov:
	( cd Tcov && $(MAKE) )

# just compile after running cmake; do not run cmake again
remake:
	( cd build && cmake --build . --config Release -j${JOBS} )

# just run cmake to set things up
setup:
	( cd build && cmake $(CMAKE_OPTIONS) .. )

install:
	( cd build && cmake --install . )

# remove any installed libraries and #include files
uninstall:
	- xargs rm < build/install_manifest.txt

# remove all files not in the distribution
clean:
	- $(RM) -rf build/* Config/*.tmp MATLAB/*.o MATLAB/*.mex* timelog.m
	- $(RM) -rf MATLAB/*/*.o MATLAB/*/*.mex*
	( cd Tcov && $(MAKE) purge )

purge: clean

distclean: clean

docs:
	( cd Doc && $(MAKE) )

