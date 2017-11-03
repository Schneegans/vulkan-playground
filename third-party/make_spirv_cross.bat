@echo off

rem create some required variables -----------------------------------------------

set SCRIPT_DIR=%~dp0

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

set SOURCE_DIR=%SCRIPT_DIR%\spirv-cross
set SOURCE_INSTALL_DIR=%SCRIPT_DIR%\install\windows\%BUILD_TYPE%\src
set HEADER_INSTALL_DIR=%SCRIPT_DIR%\install\windows\%BUILD_TYPE%\include\vulkan

rem create build directory if neccessary -----------------------------------------

if exist "%SOURCE_INSTALL_DIR%" goto SOURCE_INSTALL_DIR_CREATED
    mkdir "%SOURCE_INSTALL_DIR%"
:SOURCE_INSTALL_DIR_CREATED

if exist "%HEADER_INSTALL_DIR%" goto HEADER_INSTALL_DIR_CREATED
    mkdir "%HEADER_INSTALL_DIR%"
:HEADER_INSTALL_DIR_CREATED

rem configure cmake --------------------------------------------------------------

COPY /v "%SOURCE_DIR%\spirv*.cpp" "%SOURCE_INSTALL_DIR%"

COPY /v "%SOURCE_DIR%\GLSL.std.450.h" "%HEADER_INSTALL_DIR%"
COPY /v "%SOURCE_DIR%\spirv*.hpp" "%HEADER_INSTALL_DIR%"

@echo on
