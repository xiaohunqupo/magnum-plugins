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
set(CMAKE_FOLDER "MagnumPlugins/BcDecImageConverter/Test")

if(CORRADE_TARGET_EMSCRIPTEN OR CORRADE_TARGET_ANDROID)
    set(BCDECIMAGECONVERTER_TEST_DIR ".")
    set(BASISIMPORTER_TEST_DIR ".")
    set(DDSIMPORTER_TEST_DIR ".")
    set(KTXIMPORTER_TEST_DIR ".")
    set(PNGIMPORTER_TEST_DIR ".")
    set(STBIMAGEIMPORTER_TEST_DIR ".")
else()
    set(BCDECIMAGECONVERTER_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(BASISIMPORTER_TEST_DIR ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/BasisImporter/Test)
    set(DDSIMPORTER_TEST_DIR ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/DdsImporter/Test)
    set(KTXIMPORTER_TEST_DIR ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/KtxImporter/Test)
    set(PNGIMPORTER_TEST_DIR ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/PngImporter/Test)
    set(STBIMAGEIMPORTER_TEST_DIR ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/StbImageImporter/Test)
endif()

find_package(Magnum REQUIRED DebugTools)

if(NOT MAGNUM_BCDECIMAGECONVERTER_BUILD_STATIC)
    set(BCDECIMAGECONVERTER_PLUGIN_FILENAME $<TARGET_FILE:BcDecImageConverter>)
    if(MAGNUM_WITH_DDSIMPORTER)
        set(DDSIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:DdsImporter>)
    endif()
    if(MAGNUM_WITH_KTXIMPORTER)
        set(KTXIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:KtxImporter>)
    endif()
    if(MAGNUM_WITH_STBIMAGEIMPORTER)
        set(STBIMAGEIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:StbImageImporter>)
    endif()
endif()

# First replace ${} variables, then $<> generator expressions
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/configure.h
    INPUT ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)

corrade_add_test(BcDecImageConverterTest BcDecImageConverterTest.cpp
    LIBRARIES Magnum::Trade Magnum::DebugTools
    FILES
        bc6h.dds
        bc6hs.dds
        rgb16f.ktx2
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/BasisImporter/Test/rgba-64x32.png
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/BasisImporter/Test/rgba-63x27.png
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/DdsImporter/Test/dxt1.dds
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/DdsImporter/Test/dxt3.dds
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/DdsImporter/Test/dxt3-incomplete-blocks.dds
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/DdsImporter/Test/dxt5.dds
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/DdsImporter/Test/bc4unorm.dds
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/DdsImporter/Test/bc4snorm.dds
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/DdsImporter/Test/bc5unorm.dds
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/DdsImporter/Test/bc5snorm.dds
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/DdsImporter/Test/dxt10-bc7.dds
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/KtxImporter/Test/2d-compressed-bc1.ktx2
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/KtxImporter/Test/pattern-pot.png
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/PngImporter/Test/rgb.png
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/StbImageImporter/Test/rgb.hdr)
target_include_directories(BcDecImageConverterTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
if(MAGNUM_BCDECIMAGECONVERTER_BUILD_STATIC)
    target_link_libraries(BcDecImageConverterTest PRIVATE BcDecImageConverter)
    if(MAGNUM_WITH_DDSIMPORTER)
        target_link_libraries(BcDecImageConverterTest PRIVATE DdsImporter)
    endif()
    if(MAGNUM_WITH_KTXIMPORTER)
        target_link_libraries(BcDecImageConverterTest PRIVATE KtxImporter)
    endif()
    if(MAGNUM_WITH_STBIMAGEIMPORTER)
        target_link_libraries(BcDecImageConverterTest PRIVATE StbImageImporter)
    endif()
else()
    # So the plugins get properly built when building the test
    add_dependencies(BcDecImageConverterTest BcDecImageConverter)
    if(MAGNUM_WITH_DDSIMPORTER)
        add_dependencies(BcDecImageConverterTest DdsImporter)
    endif()
    if(MAGNUM_WITH_KTXIMPORTER)
        add_dependencies(BcDecImageConverterTest KtxImporter)
    endif()
    if(MAGNUM_WITH_STBIMAGEIMPORTER)
        add_dependencies(BcDecImageConverterTest StbImageImporter)
    endif()
endif()
if(CORRADE_BUILD_STATIC AND NOT MAGNUM_BCDECIMAGECONVERTER_BUILD_STATIC)
    # CMake < 3.4 does this implicitly, but 3.4+ not anymore (see CMP0065).
    # That's generally okay, *except if* the build is static, the executable
    # uses a plugin manager and needs to share globals with the plugins (such
    # as output redirection and so on).
    set_target_properties(BcDecImageConverterTest PROPERTIES ENABLE_EXPORTS ON)
endif()
