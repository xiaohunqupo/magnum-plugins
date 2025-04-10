#
#   This file is part of Magnum.
#
#   Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
#               2020, 2021, 2022, 2023, 2024, 2025
#             Vladimír Vondruš <mosra@centrum.cz>
#   Copyright © 2015 Jonathan Hale <squareys@googlemail.com>
#
#   Permission is hereby granted, free of charge, to any person obtaining a
#   copy of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#   DEALINGS IN THE SOFTWARE.
#

find_package(Magnum REQUIRED Audio)

if(MAGNUM_BUILD_PLUGINS_STATIC AND NOT DEFINED MAGNUM_STBVORBISAUDIOIMPORTER_BUILD_STATIC)
    set(MAGNUM_STBVORBISAUDIOIMPORTER_BUILD_STATIC 1)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

# StbVorbisAudioImporter plugin
add_plugin(StbVorbisAudioImporter
    audioimporters
    "${MAGNUM_PLUGINS_AUDIOIMPORTER_DEBUG_BINARY_INSTALL_DIR};${MAGNUM_PLUGINS_AUDIOIMPORTER_DEBUG_LIBRARY_INSTALL_DIR}"
    "${MAGNUM_PLUGINS_AUDIOIMPORTER_RELEASE_BINARY_INSTALL_DIR};${MAGNUM_PLUGINS_AUDIOIMPORTER_RELEASE_LIBRARY_INSTALL_DIR}"
    StbVorbisImporter.conf
    StbVorbisImporter.cpp
    StbVorbisImporter.h)
if(MAGNUM_STBVORBISAUDIOIMPORTER_BUILD_STATIC AND MAGNUM_BUILD_STATIC_PIC)
    set_target_properties(StbVorbisAudioImporter PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
target_include_directories(StbVorbisAudioImporter PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src)
# Include the stb_*.h files as a system directory to supress warnings
target_include_directories(StbVorbisAudioImporter SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/src/external/stb)
target_link_libraries(StbVorbisAudioImporter PUBLIC Magnum::Audio)
# On Emscripten 3.1.42 to 3.1.45 with LTO enabled, linking fails with with the
# following due to ldexp() used by stb_vorbis (which then calls into scalbnf()
# in musl). Fixed in 3.1.46.
#   wasm-ld: error: /path/to/.emscripten_cache/sysroot/lib/wasm32-emscripten/lto/libc.a(scalbnf.o): attempt to add bitcode file after LTO.
# Workaround from https://github.com/emscripten-core/emscripten/issues/16836,
# same code is in FindMagnumPlugins.
if(CORRADE_TARGET_EMSCRIPTEN AND NOT EMSCRIPTEN_VERSION VERSION_LESS 3.1.42 AND EMSCRIPTEN_VERSION VERSION_LESS 3.1.46)
    if(CMAKE_VERSION VERSION_LESS 3.13)
        message(FATAL_ERROR "CMake 3.13+ is required in order to specify Emscripten linker options")
    endif()
    target_link_options(StbVorbisAudioImporter PUBLIC $<$<CONFIG:Release>:-Wl,-u,scalbnf>)
endif()

install(FILES StbVorbisImporter.h ${CMAKE_CURRENT_BINARY_DIR}/configure.h
    DESTINATION ${MAGNUM_PLUGINS_INCLUDE_INSTALL_DIR}/StbVorbisAudioImporter)

# Automatic static plugin import
if(MAGNUM_STBVORBISAUDIOIMPORTER_BUILD_STATIC)
    install(FILES importStaticPlugin.cpp DESTINATION ${MAGNUM_PLUGINS_INCLUDE_INSTALL_DIR}/StbVorbisAudioImporter)
    target_sources(StbVorbisAudioImporter INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/importStaticPlugin.cpp)
endif()

if(MAGNUM_BUILD_TESTS)
    add_subdirectory(Test ${EXCLUDE_FROM_ALL_IF_TEST_TARGET})
endif()

# MagnumPlugins StbVorbisAudioImporter target alias for superprojects
add_library(MagnumPlugins::StbVorbisAudioImporter ALIAS StbVorbisAudioImporter)
