# Copyright MediaZ Teknoloji A.S. All Rights Reserved.
cmake_minimum_required(VERSION 3.24.2)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)

if (NOT WITH_NODOS_WORKSPACE)
    message(FATAL_ERROR "This repo currently does not support builds without Nodos workspace. "
    "Place this repo under nodos-workspace/Module folder and run cmake -S ./Toolchain/CMake -B Build from workspace root.")
endif()

add_compile_definitions(NOS_DISABLE_DEPRECATED)
# Nodos SDK
nos_find_sdk("1.3.0" NOS_PLUGIN_SDK_TARGET NOS_SUBSYSTEM_SDK_TARGET NOS_SDK_DIR)
set(FLATC_EXECUTABLE ${NOS_SDK_DIR}/bin/flatc${CMAKE_EXECUTABLE_SUFFIX})

# AJA
# ---
# Caution: AJA wants C++11 standard. Change it back to our's after processing AJA
set(CMAKE_CXX_STANDARD 11)

set(AJANTV2_DISABLE_DEMOS ON)
set(AJANTV2_DISABLE_DRIVER ON)
set(AJANTV2_DISABLE_TESTS ON)
set(AJANTV2_DISABLE_TOOLS ON)
set(AJANTV2_DISABLE_PLUGIN_LOAD ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
#if(WIN32)
	remove_definitions(-DNOMINMAX)
#endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/External/libajantv2 "${CMAKE_CURRENT_BINARY_DIR}/ajalibraries")

nos_get_targets(NOSAJA_EXTERNAL_TARGETS "./External/libajantv2")
nos_group_targets("${NOSAJA_EXTERNAL_TARGETS}" "External")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_DEBUG_POSTFIX "")

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")

# nos.sys.vulkan
nos_get_module("nos.sys.vulkan" "6.0" NOS_SYS_VULKAN_TARGET)

# nos.mediaio
nos_get_module("nos.mediaio" "2.5" NOS_MEDIAIO_TARGET)
nos_find_module_path("nos.mediaio" "2.5" NOS_MEDIAIO_PATH)
set(NOS_MEDIAIO_GENERATED_DIR "${SOURCE_DIR}/__generated__/nos.mediaio")
nos_generate_flatbuffers("${NOS_MEDIAIO_PATH}/Config" "${NOS_MEDIAIO_GENERATED_DIR}" "cpp" "${NOS_MEDIAIO_PATH};${NOS_SDK_DIR}/types" generated_nosAJA_dep_nosMediaIO)

# nos.utilities
nos_get_module("nos.utilities" "3.8" NOS_UTILITIES_TARGET)
nos_find_module_path("nos.utilities" "3.8" NOS_UTILITIES_PATH)
set(NOS_UTILITIES_GENERATED_DIR "${SOURCE_DIR}/__generated__/nos.utilities")
nos_generate_flatbuffers("${NOS_UTILITIES_PATH}/Config" "${NOS_UTILITIES_GENERATED_DIR}" "cpp" "${NOS_UTILITIES_PATH};${NOS_SDK_DIR}/types" generated_nosAJA_dep_nosUtilities)

# nos.sys.device
nos_get_module("nos.sys.device" "0.3" NOS_SYS_DEVICE_TARGET)

# AJA Plugin
# ----------
set(GENERATED_DIR "${SOURCE_DIR}/__generated__")
nos_generate_flatbuffers("${CMAKE_CURRENT_SOURCE_DIR}/Config" "${GENERATED_DIR}" "cpp" 
    "${NOS_UTILITIES_PATH};${NOS_MEDIAIO_PATH};${NOS_SDK_DIR}/types" nosAJA_generated)

list(APPEND DEPENDENCIES generated_nosAJA_dep_nosMediaIO generated_nosAJA_dep_nosUtilities nosAJA_generated ajantv2
    ${NOS_SYS_VULKAN_TARGET}
    ${NOS_SYS_DEVICE_TARGET}
    ${NOS_PLUGIN_SDK_TARGET})
list(APPEND INCLUDE_FOLDERS
    ${EXTERNAL_DIR}
    ${SOURCE_DIR}
    ${GENERATED_DIR}
    ${NOS_UTILITIES_GENERATED_DIR}
    ${NOS_MEDIAIO_GENERATED_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/External
    ${CMAKE_CURRENT_LIST_DIR}/External/libajantv2
#   ${CMAKE_CURRENT_LIST_DIR}/External/libajantv2/ajantv2/src/win
    ${CMAKE_CURRENT_LIST_DIR}/External/libajantv2/ajantv2/includes)

nos_add_plugin("nosAJA" "${DEPENDENCIES}" "${INCLUDE_FOLDERS}")

# Project generation
nos_group_targets("nosAJA" "NOS Plugins")