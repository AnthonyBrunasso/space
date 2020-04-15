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

if [ -z "$CXXFLAGS" ]; then
  CXXFLAGS="-O1 -fno-omit-frame-pointer -std=c++17 -ffp-contract=off -Wno-format "
fi

if [ -n "$DEV_FLAGS" ]; then
  CXXFLAGS+="$DEV_FLAGS "
fi

# Detect platform being run on.
uname_out="$(uname -s)"
case "${uname_out}" in
    Linux*)     unix=1;;
    Darwin*)    macosx=1;;
    *)          platform="UNKNOWN:${uname_out}"
esac
