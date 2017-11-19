#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

macro(CompileShaders _TARGET _SPIRV_FILES _GLSL_FILES)

  if(GLSLANGVALIDATOR)
    foreach(GLSL_FILE ${_GLSL_FILES})
      get_filename_component(FILE_NAME ${GLSL_FILE} NAME)
      set(SPIRV_FILE ${CMAKE_CURRENT_BINARY_DIR}/${FILE_NAME}.spv)
      add_custom_command(
        OUTPUT ${SPIRV_FILE}
        DEPENDS ${GLSL_FILE}
        COMMENT "Compiling ${GLSL_FILE} ..."
        COMMAND ${GLSLANGVALIDATOR} -o ${SPIRV_FILE} -V ${GLSL_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      )
      list(APPEND ${_SPIRV_FILES} ${SPIRV_FILE})

    endforeach()

    add_custom_target(${_TARGET} ALL DEPENDS ${${_SPIRV_FILES}})
  else(GLSLANGVALIDATOR)
    MESSAGE(WARNING "could not find GLSLANGVALIDATOR to compile shaders")
  endif(GLSLANGVALIDATOR)

endmacro()
