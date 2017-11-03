#!/bin/bash

# ------------------------------------------------------------------------------
# This script copies the required stb headers.
# ------------------------------------------------------------------------------

# get the location of this script
SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
BUILD_TYPE=${1:-Release}

# define locations of vulkan header and install location
SOURCE_DIR="$SCRIPT_DIR/stb"
INSTALL_DIR="$SCRIPT_DIR/install/linux/$BUILD_TYPE/include"

# create install directory if neccessary ---------------------------------------

if [ ! -d $INSTALL_DIR ]; then
  mkdir -p $INSTALL_DIR
fi

# installation -----------------------------------------------------------------

cp -v $SOURCE_DIR/stb_image.h $INSTALL_DIR
cp -v $SOURCE_DIR/stb_vorbis.c $INSTALL_DIR
