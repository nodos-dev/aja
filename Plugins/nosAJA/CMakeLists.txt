# Dependencies
# ------------
# AJA
if(WIN32)
    add_compile_definitions(NOMINMAX)
endif()

# TODO: Make External/ajalibraries a submodule (https://github.com/aja-video/ntv2)?
list(APPEND CMAKE_MODULE_PATH  ${CMAKE_CURRENT_LIST_DIR}/External/ntv2/cmake)

# Caution: AJA wants C++11 standard. Change it back to our's after processing AJA
include(AJAHelpers)
include(AJABuildOptions)
include(AJACommonDefines)
include(AJACommonFlags)
include(AJACommonPaths)
include(AJACommonVars)
include(AJAVersionConfig)
# CMake includes
include(GNUInstallDirs)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/External/ntv2/ajalibraries "${CMAKE_CURRENT_BINARY_DIR}/ajalibraries")

set(CMAKE_DEBUG_POSTFIX "")

list(APPEND DEPENDENCIES stb ajantv2)
list(APPEND INCLUDE_FOLDERS
    ${EXTERNAL_DIR}
    ${SUBSYSTEMS_SOURCE_DIR}/nosVulkanSubsystem/Include
    ${CMAKE_CURRENT_LIST_DIR}/External
    ${CMAKE_CURRENT_LIST_DIR}/External/ntv2
    ${CMAKE_CURRENT_LIST_DIR}/External/ntv2/ajalibraries/ajantv2/src/win
    ${CMAKE_CURRENT_LIST_DIR}/External/ntv2/ajalibraries/ajantv2/includes)
add_folder_as_plugin_project("nosAJA" "${DEPENDENCIES}" "${INCLUDE_FOLDERS}")
