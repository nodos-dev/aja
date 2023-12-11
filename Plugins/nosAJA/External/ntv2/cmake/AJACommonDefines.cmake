# Common preprocessor defines
add_compile_definitions(
	$<$<CONFIG:DEBUG>:AJA_DEBUG> $<$<CONFIG:DEBUG>:_DEBUG> 
	$<$<CONFIG:RELEASE>:NDEBUG> $<$<CONFIG:MINSIZEREL>:NDEBUG>)
# Platform-specific preprocessor defines
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_definitions(
        -DAJA_WINDOWS
        -DMSWindows
        -D_WINDOWS
        -D_CONSOLE
        -DUNICODE
        -D_UNICODE
        -DWIN32_LEAN_AND_MEAN
        -D_CRT_SECURE_NO_WARNINGS
        -D_SCL_SECURE_NO_WARNINGS)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(
        -DAJALinux
        -DAJA_LINUX)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_definitions(
        -DAJAMac
        -DAJA_MAC
        -D__STDC_CONSTANT_MACROS)
endif()
