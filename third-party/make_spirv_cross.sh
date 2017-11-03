#!/bin/bash

# ------------------------------------------------------------------------------
# This script copies the required spirv-cross headers and sources.
# ------------------------------------------------------------------------------

# get the location of this script
SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
BUILD_TYPE=${1:-Release}

# define locations of vulkan header and install location
SOURCE_DIR="$SCRIPT_DIR/spirv-cross"
SOURCE_INSTALL_DIR="$SCRIPT_DIR/install/linux/$BUILD_TYPE/src"
HEADER_INSTALL_DIR="$SCRIPT_DIR/install/linux/$BUILD_TYPE/include/vulkan"

# create install directory if neccessary ---------------------------------------

if [ ! -d $SOURCE_INSTALL_DIR ]; then
  mkdir -p $SOURCE_INSTALL_DIR
fi

if [ ! -d $HEADER_INSTALL_DIR ]; then
  mkdir -p $HEADER_INSTALL_DIR
fi

# installation -----------------------------------------------------------------

cp -v $SOURCE_DIR/spirv*.cpp $SOURCE_INSTALL_DIR

cp -v $SOURCE_DIR/GLSL.std.450.h $HEADER_INSTALL_DIR
cp -v $SOURCE_DIR/spirv*.hpp $HEADER_INSTALL_DIR
