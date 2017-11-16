#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

project(VulkanPlayground CXX C)
cmake_minimum_required(VERSION 2.8.0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# ----------------------------------------------------------------------------------- project config

string(TIMESTAMP BUILD_TIME "%Y-%m-%d")
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)

# -------------------------------------------------------------------------------- find dependencies
if (UNIX)
  set(THIRD_PARTY_DIR_RELEASE "${CMAKE_SOURCE_DIR}/third-party/install/linux/Release")
  set(THIRD_PARTY_DIR_DEBUG "${CMAKE_SOURCE_DIR}/third-party/install/linux/Debug")
else ()
  set(THIRD_PARTY_DIR_RELEASE "${CMAKE_SOURCE_DIR}/third-party/install/windows/Release")
  set(THIRD_PARTY_DIR_DEBUG "${CMAKE_SOURCE_DIR}/third-party/install/windows/Debug")
endif ()

include(cmake/CompileShaders.cmake)

# find vulkan
find_package(Vulkan REQUIRED)

# find X11
find_package(X11 REQUIRED)
set(X11_LIBRARIES
  ${X11_Xcursor_LIB}
  ${X11_Xinerama_LIB}
  ${X11_Xxf86vm_LIB}
  ${X11_Xrandr_LIB}
  ${X11_X11_LIB}
)

set(INCLUDE_DIRS
  "${THIRD_PARTY_DIR_RELEASE}/include"
  "${THIRD_PARTY_DIR_RELEASE}/include/vulkan"
  "${VULKAN_INCLUDE_DIRS}"
)

if (UNIX)
  set(LIBRARIES
    optimized "${THIRD_PARTY_DIR_RELEASE}/lib/libglfw3.a"
    debug "${THIRD_PARTY_DIR_DEBUG}/lib/libglfw3.a"
    "${X11_LIBRARIES}"
    "dl" "z" "pthread"
  )
else ()
  set(LIBRARIES
    optimized "${THIRD_PARTY_DIR_RELEASE}/lib/glfw3.lib"
    debug "${THIRD_PARTY_DIR_DEBUG}/lib/glfw3.lib"
    "wldap32.lib" "crypt32.lib" "winmm.lib" "ws2_32.lib"
  )
endif ()

list(APPEND LIBRARIES
  "${VULKAN_LIBRARIES}"
)

add_definitions(
  -DNOMINMAX
)

if (UNIX)
  add_definitions("-s -O3 --std=c++11 -Wall")
elseif (WIN32)
  add_definitions("/MP")
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()

add_subdirectory(src/VulkanPlayground)
add_subdirectory(examples)
