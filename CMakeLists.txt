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
nos_find_plugin_sdk("41.0" NOS_PLUGIN_SDK_TARGET NOS_SDK_DIR)

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

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/External/libajantv2 "${CMAKE_CURRENT_BINARY_DIR}/ajalibraries")

nos_get_targets(NOSAJA_EXTERNAL_TARGETS "./External/libajantv2")
nos_group_targets("${NOSAJA_EXTERNAL_TARGETS}" "External")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_DEBUG_POSTFIX "")

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")

# nos.sys.vulkan
nos_get_module("nos.sys.vulkan" "8.0" NOS_SYS_VULKAN_TARGET)
nos_find_plugin_path("nos.sys.vulkan" "8.0" NOS_SYS_VULKAN_PATH)

# nos.sys.settings
nos_get_module("nos.sys.settings" "3.0" NOS_SYS_SETTINGS_TARGET_NAME)

# nos.audio
nos_get_plugin("nos.audio" "1.0" NOS_AUDIO_TARGET)
nos_find_module_path("nos.audio" "1.0" NOS_AUDIO_PATH)
set(NOS_AUDIO_GENERATED_DIR "${SOURCE_DIR}/__generated__/nos.audio")
nos_generate_flatbuffers("${NOS_AUDIO_PATH}/Types" "${NOS_AUDIO_GENERATED_DIR}" "cpp" 
    "${NOS_SDK_DIR}/Types;${NOS_SYS_VULKAN_PATH};${NOS_AUDIO_PATH}" generated_nosAJA_dep_nosAudio)

# nos.mediaio
nos_get_module("nos.mediaio" "4.0" NOS_MEDIAIO_TARGET)
nos_find_module_path("nos.mediaio" "4.0" NOS_MEDIAIO_PATH)
set(NOS_MEDIAIO_GENERATED_DIR "${SOURCE_DIR}/__generated__/nos.mediaio")
nos_generate_flatbuffers("${NOS_MEDIAIO_PATH}/Types" "${NOS_MEDIAIO_GENERATED_DIR}" "cpp" "${NOS_MEDIAIO_PATH};${NOS_AUDIO_PATH};${NOS_SYS_VULKAN_PATH};${NOS_SDK_DIR}/Types" generated_nosAJA_dep_nosMediaIO)

# nos.sync
nos_get_module("nos.sync" "11.1" NOS_SYNC_TARGET)
nos_find_module_path("nos.sync" "11.1" NOS_SYNC_PATH)
set(NOS_SYNC_GENERATED_DIR "${SOURCE_DIR}/__generated__/nos.sync")
nos_generate_flatbuffers("${NOS_SYNC_PATH}/Types" "${NOS_SYNC_GENERATED_DIR}" "cpp" "${NOS_SYNC_PATH};${NOS_SDK_DIR}/Types" generated_nosAJA_dep_nosSync)

# nos.utilities
nos_get_module("nos.utilities" "4.0" NOS_UTILITIES_TARGET)
nos_find_module_path("nos.utilities" "4.0" NOS_UTILITIES_PATH)
set(NOS_UTILITIES_GENERATED_DIR "${SOURCE_DIR}/__generated__/nos.utilities")
nos_generate_flatbuffers("${NOS_UTILITIES_PATH}/Types" "${NOS_UTILITIES_GENERATED_DIR}" "cpp" "${NOS_UTILITIES_PATH};${NOS_SYS_VULKAN_PATH};${NOS_SYNC_PATH};${NOS_AUDIO_PATH};${NOS_SDK_DIR}/Types" generated_nosAJA_dep_nosUtilities)

# nos.sys.device
nos_get_module("nos.sys.device" "2.0" NOS_SYS_DEVICE_TARGET)

# nos.sync
nos_get_module("nos.sync" "11.1" NOS_SYNC_TARGET)

# AJA Plugin
# ----------
set(GENERATED_DIR "${SOURCE_DIR}/__generated__")
nos_generate_flatbuffers("${CMAKE_CURRENT_SOURCE_DIR}/Types" "${GENERATED_DIR}" "cpp" 
    "${NOS_UTILITIES_PATH};${NOS_MEDIAIO_PATH};${NOS_SDK_DIR}/Types" nosAJA_generated)

list(APPEND DEPENDENCIES generated_nosAJA_dep_nosMediaIO generated_nosAJA_dep_nosUtilities nosAJA_generated ajantv2 generated_nosAJA_dep_nosAudio
    ${NOS_SYS_VULKAN_TARGET}
    ${NOS_SYS_DEVICE_TARGET}
    ${NOS_SYNC_TARGET}
    ${NOS_SYS_SETTINGS_TARGET_NAME}
    ${NOS_AUDIO_TARGET}
    ${NOS_PLUGIN_SDK_TARGET})
list(APPEND INCLUDE_FOLDERS
    ${EXTERNAL_DIR}
    ${SOURCE_DIR}
    ${GENERATED_DIR}
    "${NOS_SYS_VULKAN_PATH}/Include/nosVulkanSubsystem"
    ${NOS_UTILITIES_GENERATED_DIR}
    ${NOS_MEDIAIO_GENERATED_DIR}
    ${NOS_AUDIO_GENERATED_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/External
    ${CMAKE_CURRENT_LIST_DIR}/External/libajantv2
    ${CMAKE_CURRENT_LIST_DIR}/External/libajantv2/ajantv2/includes)

nos_add_plugin("nosAJA" "${DEPENDENCIES}" "${INCLUDE_FOLDERS}")

target_compile_definitions(nosAJA PRIVATE "-DNOMINMAX")

# Project generation
nos_group_targets("nosAJA" "NOS Plugins")