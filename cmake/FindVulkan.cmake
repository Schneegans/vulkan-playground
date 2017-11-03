#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

set(_VULKAN_SDK "")
if(VULKAN_SDK AND IS_DIRECTORY "${VULKAN_SDK}")
  set(_VULKAN_SDK "${VULKAN_SDK}")
  set(_VULKAN_SDK_EXPLICIT 1)
else()
  set(_ENV_VULKAN_SDK "")
  if(DEFINED ENV{VULKAN_SDK})
    file(TO_CMAKE_PATH "$ENV{VULKAN_SDK}" _ENV_VULKAN_SDK)
  endif()
  if(_ENV_VULKAN_SDK AND IS_DIRECTORY "${_ENV_VULKAN_SDK}")
    set(_VULKAN_SDK "${_ENV_VULKAN_SDK}")
    set(_VULKAN_SDK_EXPLICIT 1)
  endif()
  unset(_ENV_VULKAN_SDK)
endif()

if(NOT DEFINED _VULKAN_SDK_EXPLICIT)
  message(FATAL_ERROR "Must specify a VULKAN_SDK value via CMake or environment variable.")
endif()

find_program(GLSLANGVALIDATOR glslangValidator PATHS
    "${VULKAN_SDK}/bin"
    "${VULKAN_SDK}/Bin"
)

find_path(VULKAN_INCLUDE_DIR "vulkan/vulkan.h" HINTS
    "${VULKAN_SDK}/include"
    "${VULKAN_SDK}/Include"
)

find_library(VULKAN_LIBRARY vulkan vulkan-1 HINTS
    "${VULKAN_SDK}/bin"
    "${VULKAN_SDK}/lib"
)

set(VULKAN_LIBRARIES ${VULKAN_LIBRARY})
set(VULKAN_INCLUDE_DIRS ${VULKAN_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vulkan DEFAULT_MSG
                                  VULKAN_LIBRARY VULKAN_INCLUDE_DIR GLSLANGVALIDATOR)

mark_as_advanced(VULKAN_INCLUDE_DIR VULKAN_LIBRARY)

if(VULKAN_FOUND)
  message(STATUS "Found Vulkan SDK in ${VULKAN_SDK}")
  message(STATUS "VULKAN_INCLUDE_DIR:          ${VULKAN_INCLUDE_DIR}")
  message(STATUS "VULKAN_LIBRARY:              ${VULKAN_LIBRARY}")
  message(STATUS "GLSLANGVALIDATOR:            ${GLSLANGVALIDATOR}")
endif()
