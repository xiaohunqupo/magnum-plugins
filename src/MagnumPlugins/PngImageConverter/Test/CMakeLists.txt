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
set(CMAKE_FOLDER "MagnumPlugins/PngImageConverter/Test")

find_package(Magnum REQUIRED DebugTools)

if(NOT MAGNUM_PNGIMAGECONVERTER_BUILD_STATIC)
    set(PNGIMAGECONVERTER_PLUGIN_FILENAME $<TARGET_FILE:PngImageConverter>)
    if(MAGNUM_WITH_PNGIMPORTER)
        set(PNGIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:PngImporter>)
    endif()
endif()

# First replace ${} variables, then $<> generator expressions
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/configure.h
    INPUT ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)

corrade_add_test(PngImageConverterTest PngImageConverterTest.cpp
    LIBRARIES
        Magnum::Trade
        Magnum::DebugTools
    FILES
        gray.png
        rgb.png
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/PngImporter/Test/rgba.png
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/PngImporter/Test/ga.png
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/PngImporter/Test/gray16.png
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/PngImporter/Test/rgb16.png)
target_include_directories(PngImageConverterTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
if(MAGNUM_PNGIMAGECONVERTER_BUILD_STATIC)
    target_link_libraries(PngImageConverterTest PRIVATE PngImageConverter)
    if(MAGNUM_WITH_PNGIMPORTER)
        target_link_libraries(PngImageConverterTest PRIVATE PngImporter)
    endif()
else()
    # So the plugins get properly built when building the test
    add_dependencies(PngImageConverterTest PngImageConverter)
    if(MAGNUM_WITH_PNGIMPORTER)
        add_dependencies(PngImageConverterTest PngImporter)
    endif()
endif()
if(CORRADE_BUILD_STATIC AND NOT MAGNUM_PNGIMAGECONVERTER_BUILD_STATIC)
    # CMake < 3.4 does this implicitly, but 3.4+ not anymore (see CMP0065).
    # That's generally okay, *except if* the build is static, the executable
    # uses a plugin manager and needs to share globals with the plugins (such
    # as output redirection and so on).
    set_target_properties(PngImageConverterTest PROPERTIES ENABLE_EXPORTS ON)
endif()
