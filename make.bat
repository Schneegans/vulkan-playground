@echo off

rem create some required variables -----------------------------------------------

set CMAKE_DIR=%~dp0
set CURRENT_DIR=%cd%

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

set BUILD_DIR=%CMAKE_DIR%\build\windows\%BUILD_TYPE%
set INSTALL_DIR=%CMAKE_DIR%\install\windows\%BUILD_TYPE%

set CEF_ROOT="C:/Users/mail/Documents/cef"
set VULKAN_SDK="C:/VulkanSDK/1.0.39.1"
set BOOST_ROOT="C:/local/boost_1_63_0"
set BOOST_LIBRARYDIR="C:/local/boost_1_63_0/lib64-msvc-14.0"

rem create build directory if neccessary -----------------------------------------

if exist "%BUILD_DIR%" goto BUILD_DIR_CREATED
    mkdir "%BUILD_DIR%"
:BUILD_DIR_CREATED

rem configure cmake --------------------------------------------------------------

cd "%BUILD_DIR%"

cmake -G "Visual Studio 14 2015 Win64" -DCEF_ROOT=%CEF_ROOT% -DVULKAN_SDK=%VULKAN_SDK%^
      -DBOOST_ROOT=%BOOST_ROOT% -DBOOST_LIBRARYDIR=%BOOST_LIBRARYDIR%^
      -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" "%CMAKE_DIR%"
msbuild INSTALL.vcxproj /p:Configuration=%BUILD_TYPE% /m:1 /v:m

cd "%CURRENT_DIR%"

@echo on
