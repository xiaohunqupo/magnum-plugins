#
#   This file is part of Magnum.
#
#   Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
#               2020, 2021, 2022, 2023, 2024, 2025
#             Vladimír Vondruš <mosra@centrum.cz>
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

find_package(Magnum REQUIRED Trade)
# Force our own FindOpenEXR module, which then delegates to the config if
# appropriate
find_package(OpenEXR REQUIRED MODULE)

if(MAGNUM_BUILD_PLUGINS_STATIC AND NOT DEFINED MAGNUM_OPENEXRIMAGECONVERTER_BUILD_STATIC)
    set(MAGNUM_OPENEXRIMAGECONVERTER_BUILD_STATIC 1)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

# OpenExrImageConverter plugin
add_plugin(OpenExrImageConverter
    imageconverters
    "${MAGNUM_PLUGINS_IMAGECONVERTER_DEBUG_BINARY_INSTALL_DIR};${MAGNUM_PLUGINS_IMAGECONVERTER_DEBUG_LIBRARY_INSTALL_DIR}"
    "${MAGNUM_PLUGINS_IMAGECONVERTER_RELEASE_BINARY_INSTALL_DIR};${MAGNUM_PLUGINS_IMAGECONVERTER_RELEASE_LIBRARY_INSTALL_DIR}"
    OpenExrImageConverter.conf
    OpenExrImageConverter.cpp
    OpenExrImageConverter.h)
if(MAGNUM_OPENEXRIMAGECONVERTER_BUILD_STATIC AND MAGNUM_BUILD_STATIC_PIC)
    set_target_properties(OpenExrImageConverter PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
# Include the files as a system directory to supress warnings
target_include_directories(OpenExrImageConverter SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/src/external/miniexr)
target_include_directories(OpenExrImageConverter PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src)
target_link_libraries(OpenExrImageConverter PUBLIC Magnum::Trade OpenEXR::OpenEXR)
# OpenEXR uses exceptions. No way around those.
# https://github.com/AcademySoftwareFoundation/openexr/issues/211
if(CORRADE_TARGET_EMSCRIPTEN)
    if(CMAKE_VERSION VERSION_LESS 3.13)
        message(FATAL_ERROR "CMake 3.13+ is required in order to specify Emscripten linker options")
    endif()
    target_compile_options(OpenExrImageConverter PUBLIC "SHELL:-s DISABLE_EXCEPTION_CATCHING=0")
    target_link_options(OpenExrImageConverter PUBLIC "SHELL:-s DISABLE_EXCEPTION_CATCHING=0")
endif()

install(FILES OpenExrImageConverter.h ${CMAKE_CURRENT_BINARY_DIR}/configure.h
    DESTINATION ${MAGNUM_PLUGINS_INCLUDE_INSTALL_DIR}/OpenExrImageConverter)

# Automatic static plugin import
if(MAGNUM_OPENEXRIMAGECONVERTER_BUILD_STATIC)
    install(FILES importStaticPlugin.cpp DESTINATION ${MAGNUM_PLUGINS_INCLUDE_INSTALL_DIR}/OpenExrImageConverter)
    target_sources(OpenExrImageConverter INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/importStaticPlugin.cpp)
endif()

if(MAGNUM_BUILD_TESTS)
    add_subdirectory(Test ${EXCLUDE_FROM_ALL_IF_TEST_TARGET})
endif()

# MagnumPlugins OpenExrImageConverter target alias for superprojects
add_library(MagnumPlugins::OpenExrImageConverter ALIAS OpenExrImageConverter)
