#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

set(SHADERS
    "color.frag"
    "color.vert"
    "texture.frag"
    "texture.vert"
)

CompileShaders(CompileDemoShaders SPIRV_FILES HEADER_FILES "${SHADERS}")

install(FILES ${SPIRV_FILES}
    DESTINATION "bin/data/shaders"
)
