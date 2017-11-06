@echo off

rem create some required variables -----------------------------------------------

set SCRIPT_DIR=%~dp0
set CURRENT_DIR=%cd%

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

set CMAKE_DIR=%SCRIPT_DIR%\gli
set BUILD_DIR=%SCRIPT_DIR%\build\windows\%BUILD_TYPE%\gli
set INSTALL_DIR=%SCRIPT_DIR%\install\windows\%BUILD_TYPE%

rem create build directory if neccessary -----------------------------------------

if exist "%BUILD_DIR%" goto BUILD_DIR_CREATED
    mkdir "%BUILD_DIR%"
:BUILD_DIR_CREATED

rem configure cmake --------------------------------------------------------------

cd "%BUILD_DIR%"

cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" "%CMAKE_DIR%"
msbuild INSTALL.vcxproj /p:Configuration=%BUILD_TYPE% /m:8 /v:m

cd "%CURRENT_DIR%"

@echo on
