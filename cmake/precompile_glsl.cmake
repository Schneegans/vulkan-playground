#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

macro(precompile_glsl _TARGET _OUTPUTS _SOURCES)

  if(GLSLANGVALIDATOR)
    foreach(SOURCE ${_SOURCES})
      set(RESULT ${CMAKE_CURRENT_BINARY_DIR}/${SOURCE}.spv)
      add_custom_command(
        OUTPUT ${RESULT}
        DEPENDS ${SOURCE}
        COMMENT "Compiling ${SOURCE} ..."
        COMMAND ${GLSLANGVALIDATOR} -o ${CMAKE_CURRENT_BINARY_DIR}/${SOURCE}.spv -V ${SOURCE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      )
      list(APPEND ${_OUTPUTS} ${RESULT})
    endforeach()

    add_custom_target(${_TARGET} ALL DEPENDS ${${_OUTPUTS}})

  else(GLSLANGVALIDATOR)
    MESSAGE(WARNING "could not find GLSLANGVALIDATOR to compile shaders")

  endif(GLSLANGVALIDATOR)

endmacro()
