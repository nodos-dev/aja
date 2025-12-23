# Copyright MediaZ Teknoloji A.S. All Rights Reserved.

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

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/External/libajantv2 "${CMAKE_CURRENT_BINARY_DIR}/ajalibraries")

nos_get_targets(NOSAJA_EXTERNAL_TARGETS "${CMAKE_CURRENT_BINARY_DIR}/ajalibraries")
nos_group_targets("${NOSAJA_EXTERNAL_TARGETS}" "External")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_DEBUG_POSTFIX "")

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")

target_link_libraries(${NOS_PLUGIN_TARGET} PRIVATE ajantv2)
target_include_directories(${NOS_PLUGIN_TARGET} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/External
    ${CMAKE_CURRENT_SOURCE_DIR}/External/libajantv2
#   ${CMAKE_CURRENT_SOURCE_DIR}/External/libajantv2/ajantv2/src/win
    ${CMAKE_CURRENT_SOURCE_DIR}/External/libajantv2/ajantv2/includes)

if (WIN32)
    target_compile_definitions(${NOS_PLUGIN_TARGET} PRIVATE -DNOMINMAX)
endif()

target_compile_definitions(${NOS_PLUGIN_TARGET} PRIVATE "NOS_DISABLE_DEPRECATED")