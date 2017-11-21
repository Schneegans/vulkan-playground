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
    "PBR.frag"
    "PBR.vert"
    "TexturedQuad.frag"
    "TexturedQuad.vert"
)

CompileShaders(CompileShaders SPIRV_FILES "${SHADERS}")

# -------------------------------------------------------------------------------- shader reflection
ExtractReflection(TexturedQuad REFLECTION_HEADERS "TexturedQuad.vert.spv" "TexturedQuad.frag.spv")
ExtractReflection(PBR REFLECTION_HEADERS "PBR.vert.spv" "PBR.frag.spv")

add_custom_target(ExtractReflection ALL DEPENDS ${REFLECTION_HEADERS})

install(FILES ${SPIRV_FILES}
    DESTINATION "bin/data/shaders"
)
