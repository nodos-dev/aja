# Common compiler flags
if (CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP /Zm200 /Zc:wchar_t-")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /Zm200 /Zc:wchar_t-")
    if(CMAKE_BUILD_TYPE MATCHES "^(Debug|RelWithDebInfo)$")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Od /RTC1 /W3")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Od /RTC1 /W3")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /O2 /W3")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /W3")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "^((Apple)Clang|GNU)$")
    if (CMAKE_SYSTEM_NAME MATCHES "^(Linux|Darwin)$")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    endif()
    if(CMAKE_BUILD_TYPE MATCHES "^(Debug|RelWithDebInfo)$")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O0 -g")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0 -g")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wextra")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra")
    endif()
endif()
