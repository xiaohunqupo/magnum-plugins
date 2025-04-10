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

# IDE folder in VS, Xcode etc. CMake 3.12+, older versions have only the FOLDER
# property that would have to be set on each target separately.
set(CMAKE_FOLDER "MagnumPlugins/OpenExrImporter/Test")

# See OpenExrImporter.h for details -- the plugin itself can't be linked to
# pthread, the app has to be instead. As of 2022-01-28, the CI nondeprecated
# (w/o asserts, Release) build started mysteriously failing unless
# THREADS_PREFER_PTHREAD_FLAG is set, with -lpthread doing nothing. See
# BasisImageConverter/Test/CMakeLists.txt for further details.
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

if(CORRADE_TARGET_EMSCRIPTEN OR CORRADE_TARGET_ANDROID)
    set(OPENEXRIMPORTER_TEST_DIR ".")
else()
    set(OPENEXRIMPORTER_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(NOT MAGNUM_OPENEXRIMPORTER_BUILD_STATIC)
    set(OPENEXRIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:OpenExrImporter>)
endif()

# First replace ${} variables, then $<> generator expressions
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/configure.h
    INPUT ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)

corrade_add_test(OpenExrImporterTest OpenExrImporterTest.cpp
    LIBRARIES
        Magnum::Trade
        # See OpenExrImporter.h for details -- the plugin itself can't be
        # linked to pthread, the app has to be instead
        Threads::Threads
    FILES
        # Most of these produced directly by OpenExrImageConverter
        depth32f.exr
        depth32f-custom-channels.exr
        rg32ui.exr
        rgb16f.exr
        rgb16f-custom-windows.exr
        rgb16f-tiled.exr
        # exrmaketiled -r rgb16f.exr rgb16f-ripmap.exr
        rgb16f-ripmap.exr
        rgba32f.exr
        rgba32f-custom-channels.exr
        # This one edited from rgba32f.exr by changing A channel type to UINT
        rgb32fa32ui.exr
        # This one edited from depth32f.exr by changing Z channel type to UINT
        depth32ui.exr

        envmap-cube.exr
        envmap-cube-custom-windows.exr

        levels2D.exr
        levels2D-tile1x1.exr
        levels2D-incomplete.exr
        levels-cube.exr
        levels-cube-incomplete.exr)
target_include_directories(OpenExrImporterTest PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>
    $<TARGET_PROPERTY:OpenEXR::OpenEXR,INTERFACE_INCLUDE_DIRECTORIES>)
if(MAGNUM_OPENEXRIMPORTER_BUILD_STATIC)
    target_link_libraries(OpenExrImporterTest PRIVATE OpenExrImporter)
else()
    # So the plugins get properly built when building the test
    add_dependencies(OpenExrImporterTest OpenExrImporter)
endif()
if(CORRADE_BUILD_STATIC AND NOT MAGNUM_OPENEXRIMPORTER_BUILD_STATIC)
    # CMake < 3.4 does this implicitly, but 3.4+ not anymore (see CMP0065).
    # That's generally okay, *except if* the build is static, the executable
    # uses a plugin manager and needs to share globals with the plugins (such
    # as output redirection and so on).
    set_target_properties(OpenExrImporterTest PROPERTIES ENABLE_EXPORTS ON)
endif()
