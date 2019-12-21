#!/bin/bash
BIN_DIR="bin"

# Make the bin directory if it doesn't already exist.
mkdir $BIN_DIR -p

# detect compiler for c++
if [ -z "$CXX" ]; then
	CXX=`which clang++ || which g++`
fi
if [ -z "$CXX" ]; then
	echo "Failed to detect C++ compiler"
	exit 1
fi

# detect compiler for C
if [ -z "$CC" ]; then
	CC=`which clang || which gcc`
fi
if [ -z "$CC" ]; then
	echo "Failed to detect C compiler"
	exit 1
fi
