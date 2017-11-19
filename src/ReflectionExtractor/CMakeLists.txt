#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

add_executable(ReflectionExtractor "main.cpp")

target_include_directories(ReflectionExtractor
  PRIVATE "${CMAKE_SOURCE_DIR}/src"
  PUBLIC ${INCLUDE_DIRS}
)

target_link_libraries(ReflectionExtractor VulkanPlayground)
