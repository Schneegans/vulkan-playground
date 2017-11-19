#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

# ----------------------------------------------------------------------------- generate spirv files
set(SHADERS
    "color.frag"
    "color.vert"
    "texture.frag"
    "texture.vert"
)

CompileShaders(CompileShaders SPIRV_FILES "${SHADERS}")

# -------------------------------------------------------------------------------- shader reflection
ExtractReflection(SimpleTexture REFLECTION_HEADERS "texture.vert.spv" "texture.frag.spv")
ExtractReflection(SimpleColor REFLECTION_HEADERS "color.vert.spv" "color.frag.spv")

add_custom_target(ExtractReflection ALL DEPENDS ${REFLECTION_HEADERS})

install(FILES ${SPIRV_FILES}
    DESTINATION "bin/data/shaders"
)
