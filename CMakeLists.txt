cmake_minimum_required(VERSION 3.16)
project(BITWAVE VERSION 0.1.0 LANGUAGES C)

if(PROJECT_IS_TOP_LEVEL)
    set(BITWAVE_EXAMPLES_DEFAULT ON)
else()
    set(BITWAVE_EXAMPLES_DEFAULT OFF)
endif()

option(BITWAVE_BUILD_EXAMPLES  "Build the BITWAVE examples"                    ${BITWAVE_EXAMPLES_DEFAULT})
option(BITWAVE_APPLE_STATIC_LINK "Link Apple frameworks directly (notarization friendly)" OFF)

add_library(bitwave
    bitwave.c
    deps/miniaudio.c
)
add_library(bitwave::bitwave ALIAS bitwave)

target_include_directories(bitwave
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(bitwave PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

if(BUILD_SHARED_LIBS)
    if(WIN32)
        set_target_properties(bitwave PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif()
else()
    set_target_properties(bitwave PROPERTIES DEFINE_SYMBOL BITWAVE_STATIC)
endif()

if(MSVC)
    target_compile_options(bitwave PRIVATE /utf-8)
endif()

# miniaudio resolves Windows APIs (avrt, COM, winmm) at runtime, so Windows
# needs no extra system libraries. Everywhere else libm/dl are required.
if(NOT WIN32)
    target_link_libraries(bitwave PUBLIC m)
    if(CMAKE_DL_LIBS)
        target_link_libraries(bitwave PUBLIC ${CMAKE_DL_LIBS})
    endif()
endif()

# Apple runtime linking uses dlopen, which can break notarization.
# MA_NO_RUNTIME_LINKING links the frameworks at build time instead.
if(BITWAVE_APPLE_STATIC_LINK)
    if(NOT APPLE)
        message(WARNING "BITWAVE_APPLE_STATIC_LINK only affects Apple platforms")
    endif()
    target_compile_definitions(bitwave PRIVATE MA_NO_RUNTIME_LINKING)
    target_link_libraries(bitwave PUBLIC
        "-framework CoreFoundation"
        "-framework CoreAudio"
        "-framework AudioToolbox"
    )
endif()

find_package(Threads REQUIRED)
target_link_libraries(bitwave PUBLIC Threads::Threads)

if(BITWAVE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
