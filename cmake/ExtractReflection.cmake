#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

macro(ExtractReflection _NAME _HPP_FILES)

  set(_SPIRV_FILES)

  foreach(SPIRV_FILE ${ARGN})
    set(_SPIRV_FILES ${_SPIRV_FILES} ${CMAKE_CURRENT_BINARY_DIR}/${SPIRV_FILE})
  endforeach()

  set(HPP_FILE ${CMAKE_CURRENT_BINARY_DIR}/${_NAME}.hpp)

  add_custom_command(
    OUTPUT ${HPP_FILE}
    DEPENDS ${ARGN}
    COMMENT "Extracting reflection information from ${ARGN} ..."
    COMMAND ReflectionExtractor ${_SPIRV_FILES} ${_NAME} ${HPP_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
  list(APPEND ${_HPP_FILES} ${HPP_FILE})

endmacro()
