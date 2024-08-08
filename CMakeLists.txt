# Copyright MediaZ Teknoloji A.S. All Rights Reserved.
cmake_minimum_required(VERSION 3.24.2)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)

if (NOT WITH_NODOS_WORKSPACE)
    message(FATAL_ERROR "This repo currently does not support builds without Nodos workspace. "
    "Place this repo under nodos-workspace/Module folder and run cmake -S ./Toolchain/CMake -B Build from workspace root.")
endif()

# Nodos SDK
nos_find_sdk("1.2.0" NOS_PLUGIN_SDK_TARGET NOS_SUBSYSTEM_SDK_TARGET NOS_SDK_DIR)
set(FLATC_EXECUTABLE ${NOS_SDK_DIR}/bin/flatc)

# Dependencies
# ------------
nos_get_module("nos.sys.vulkan" "5.7" NOS_SYS_VULKAN_TARGET_5_7)

# AJA
# ---
# Caution: AJA wants C++11 standard. Change it back to our's after processing AJA
set(CMAKE_CXX_STANDARD 11)

set(AJANTV2_DISABLE_DEMOS ON)
set(AJANTV2_DISABLE_DRIVER ON)
set(AJANTV2_DISABLE_TESTS ON)
set(AJANTV2_DISABLE_TOOLS ON)
set(AJANTV2_DISABLE_PLUGINS ON)

#if(WIN32)
	remove_definitions(-DNOMINMAX)
#endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/External/libajantv2 "${CMAKE_CURRENT_BINARY_DIR}/ajalibraries")

nos_get_targets(NOSAJA_EXTERNAL_TARGETS "./External/libajantv2")
nos_group_targets("${NOSAJA_EXTERNAL_TARGETS}" "External")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_DEBUG_POSTFIX "")

# nos.sys.vulkan
nos_get_module("nos.sys.vulkan" "5.7" NOS_SYS_VULKAN_TARGET_5_7)

# nos.mediaio
nos_get_module("nos.mediaio" "1.3" NOS_MEDIAIO_TARGET_1_3)
nos_find_module_path("nos.mediaio" "1.3" NOS_MEDIAIO_PATH_1_3)
nos_generate_flatbuffers("${NOS_MEDIAIO_PATH_1_3}/Config" "${CMAKE_CURRENT_SOURCE_DIR}/Source" "cpp" "${NOS_MEDIAIO_PATH_1_3};${NOS_SDK_DIR}/types" nosMediaIO_1_3_generated)

# nos.utilities
nos_get_module("nos.utilities" "2.3" NOS_UTILITIES_TARGET_2_3)
nos_find_module_path("nos.utilities" "2.3" NOS_UTILITIES_PATH_2_3)
nos_generate_flatbuffers("${NOS_UTILITIES_PATH_2_3}/Config" "${CMAKE_CURRENT_SOURCE_DIR}/Source" "cpp" "${NOS_UTILITIES_PATH_2_3};${NOS_SDK_DIR}/types" nosUtilities_2_3_generated)

# AJA Plugin
# ----------
nos_generate_flatbuffers("${CMAKE_CURRENT_SOURCE_DIR}/Config" "${CMAKE_CURRENT_SOURCE_DIR}/Source" "cpp" 
    "${NOS_UTILITIES_PATH_2_3};${NOS_MEDIAIO_PATH_1_3};${NOS_SDK_DIR}/types" nosAJA_generated)

list(APPEND DEPENDENCIES nosMediaIO_1_3_generated nosUtilities_2_3_generated nosAJA_generated ajantv2 ${NOS_SYS_VULKAN_TARGET_5_7} ${NOS_PLUGIN_SDK_TARGET})
list(APPEND INCLUDE_FOLDERS
    ${EXTERNAL_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_CURRENT_LIST_DIR}/External
    ${CMAKE_CURRENT_LIST_DIR}/External/libajantv2
#   ${CMAKE_CURRENT_LIST_DIR}/External/libajantv2/ajantv2/src/win
    ${CMAKE_CURRENT_LIST_DIR}/External/libajantv2/ajantv2/includes)

nos_add_plugin("nosAJA" "${DEPENDENCIES}" "${INCLUDE_FOLDERS}")

# Project generation
nos_group_targets("nosAJA" "NOS Plugins")