#!/bin/bash

# ------------------------------------------------------------------------------
# This script builds all dependencies.
# ------------------------------------------------------------------------------


# get the location of this script
SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"

# update all sumodules
cd $SCRIPT_DIR
git submodule update --init --recursive

echo ""
echo "-------------------------------------------------------------------------"
echo "------------------------- Building GLFW ---------------------------------"
echo "-------------------------------------------------------------------------"
echo ""
$SCRIPT_DIR/third-party/make_glfw.sh Release
$SCRIPT_DIR/third-party/make_glfw.sh Debug

echo ""
echo "-------------------------------------------------------------------------"
echo "-------------------------- Building GLI ---------------------------------"
echo "-------------------------------------------------------------------------"
echo ""
$SCRIPT_DIR/third-party/make_gli.sh Release
$SCRIPT_DIR/third-party/make_gli.sh Debug

echo ""
echo "-------------------------------------------------------------------------"
echo "------------------------- Installing GLM --------------------------------"
echo "-------------------------------------------------------------------------"
echo ""
$SCRIPT_DIR/third-party/make_glm.sh Release
$SCRIPT_DIR/third-party/make_glm.sh Debug

echo ""
echo "-------------------------------------------------------------------------"
echo "---------------------- Building SPIRV-Cross -----------------------------"
echo "-------------------------------------------------------------------------"
echo ""
$SCRIPT_DIR/third-party/make_spirv_cross.sh Release
$SCRIPT_DIR/third-party/make_spirv_cross.sh Debug
