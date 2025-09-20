cmake_minimum_required(VERSION 3.0...3.20)
project(sdlcross C)

option(WITH_IMAGE "Enable SDL3_image" OFF)
option(WITH_MIXER "Enable SDL3_mixer" OFF)
option(WITH_TTF "Enable SDL3_ttf" OFF)
option(WITH_NET "Enable SDL3_net" OFF)

find_package(SDL3 REQUIRED CONFIG)

if(WITH_IMAGE)
    find_package(SDL3_image CONFIG REQUIRED)
endif()

if(WITH_MIXER)
    find_package(SDL3_mixer CONFIG REQUIRED)
endif()

if(WITH_TTF)
    find_package(SDL3_ttf CONFIG REQUIRED)
endif()

if(WITH_NET)
    find_package(SDL3_net CONFIG REQUIRED)
endif()

if(ANDROID)
    enable_language(CXX)
    add_library(sdlcross SHARED src/main.c)
    set_target_properties(sdlcross PROPERTIES LINKER_LANGUAGE CXX)
else()
    add_executable(sdlcross src/main.c)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    target_link_options("sdlcross" PRIVATE "SHELL:--embed-file ${CMAKE_CURRENT_SOURCE_DIR}/app/src/main/assets/sprites/crate.png@/app/src/main/assets/sprites/crate.png")
    set_property(TARGET "sdlcross" APPEND PROPERTY LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/app/src/main/assets/sprites/crate.png")

    target_link_options("sdlcross" PRIVATE "SHELL:--embed-file ${CMAKE_CURRENT_SOURCE_DIR}/app/src/main/assets/audio/picked-coin-echo-2.wav@/app/src/main/assets/audio/picked-coin-echo-2.wav")
    set_property(TARGET "sdlcross" APPEND PROPERTY LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/app/src/main/assets/audio/picked-coin-echo-2.wav")

    target_link_options("sdlcross" PRIVATE "SHELL:--embed-file ${CMAKE_CURRENT_SOURCE_DIR}/app/src/main/assets/fonts/arial.ttf@/app/src/main/assets/fonts/arial.ttf")
    set_property(TARGET "sdlcross" APPEND PROPERTY LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/app/src/main/assets/fonts/arial.ttf")
endif()

if(WITH_IMAGE)
    target_compile_definitions(sdlcross PRIVATE WITH_IMAGE)
    target_link_libraries(sdlcross PRIVATE SDL3_image::SDL3_image)
endif()

if(WITH_MIXER)
    target_compile_definitions(sdlcross PRIVATE WITH_MIXER)
    target_link_libraries(sdlcross PRIVATE SDL3_mixer::SDL3_mixer)
endif()

if(WITH_TTF)
    target_compile_definitions(sdlcross PRIVATE WITH_TTF)
    target_link_libraries(sdlcross PRIVATE SDL3_ttf::SDL3_ttf)
endif()

if(WITH_NET)
    target_compile_definitions(sdlcross PRIVATE WITH_NET)
    target_link_libraries(sdlcross PRIVATE SDL3_net::SDL3_net)
endif()

target_link_libraries(sdlcross PRIVATE SDL3::SDL3)

set_target_properties(sdlcross PROPERTIES
    CXX_STANDARD 11
)
if(NOT MSVC)
    target_compile_options(sdlcross PRIVATE -Wall -Wextra -Werror)
endif()
