#!/bin/bash

#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#                                                                                                  #
# This script can be used to compile and test the library and the examples. There are several      #
# possible arguments:                                                                              #
#   Release    - perform a build in release mode (default)                                         #
#   Debug      - perform a build in debug mode (only one build is performed; if Release and Debug  #
#                are provided, a debug build will be done)                                         #
#   Install    - install the compilation result to install/linux/$BUILD_TYPE                       #
#   Doc        - create the doxygen documention, will be installed to                              #
#                install/linux/$BUILD_TYPE/share/doc if Install is provided as well                #
#   Test       - executes the unit tests, requires Install                                         #
#--------------------------------------------------------------------------------------------------#

# get the location of this script
CMAKE_DIR="$( cd "$( dirname "$0" )" && pwd )"

ARGS=( "$@" )
BUILD_TYPE="Release"

# set build type to debug if Debug is one of the arguments
if [[ " ${ARGS[@]} " =~ "Debug" ]]; then
    BUILD_TYPE="Debug"
fi

# define locations for build and install
BUILD_DIR="$CMAKE_DIR/build/linux/$BUILD_TYPE"
INSTALL_DIR="$CMAKE_DIR/install/linux/$BUILD_TYPE"

# create build directory if neccessary
if [ ! -d $BUILD_DIR ]; then
  mkdir -p $BUILD_DIR
fi

# configure cmake
cd $BUILD_DIR
cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR -DCMAKE_BUILD_TYPE=$BUILD_TYPE $CMAKE_DIR

# compile if requested
make -j install
