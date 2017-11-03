#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

set(TARGET_NAME VulkanPlayground)

# ------------------------------------------------------------------------------------------ compile
file(GLOB_RECURSE FILES_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "*.cpp"
  "${THIRD_PARTY_DIR_RELEASE}/src/*.cpp"
)

include_directories(BEFORE SYSTEM
  ${INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(${TARGET_NAME} STATIC ${FILES_SRC})

target_link_libraries(${TARGET_NAME} ${LIBRARIES})

# ------------------------------------------------------------------------------------------ install
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION "include/VulkanPlayground"
  FILES_MATCHING PATTERN "*.hpp"
  PATTERN "internal" EXCLUDE)

install(TARGETS ${TARGET_NAME}
  RUNTIME DESTINATION "bin"
  LIBRARY DESTINATION "lib"
  ARCHIVE DESTINATION "lib"
)
