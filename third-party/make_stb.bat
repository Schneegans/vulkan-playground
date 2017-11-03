@echo off

rem create some required variables -----------------------------------------------

set SCRIPT_DIR=%~dp0

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

set SOURCE_DIR=%SCRIPT_DIR%\stb
set INSTALL_DIR=%SCRIPT_DIR%\install\windows\%BUILD_TYPE%\include

rem create build directory if neccessary -----------------------------------------

if exist "%INSTALL_DIR%" goto INSTALL_DIR_CREATED
    mkdir "%INSTALL_DIR%"
:INSTALL_DIR_CREATED

rem copy headers -----------------------------------------------------------------

COPY /v "%SOURCE_DIR%\stb_image.h" "%INSTALL_DIR%"
COPY /v "%SOURCE_DIR%\stb_vorbis.c" "%INSTALL_DIR%"

@echo on
