#
#   This file is part of Magnum.
#
#   Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
#               2020, 2021, 2022, 2023, 2024, 2025
#             Vladimír Vondruš <mosra@centrum.cz>
#   Copyright © 2019 Jonathan Hale <squareys@googlemail.com>
#   Copyright © 2021, 2024 Pablo Escobar <mail@rvrs.in>
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
set(CMAKE_FOLDER "MagnumPlugins/BasisImporter/Test")

find_package(Magnum REQUIRED
    COMPONENTS DebugTools
    OPTIONAL_COMPONENTS AnyImageImporter)

if(CORRADE_TARGET_EMSCRIPTEN OR CORRADE_TARGET_ANDROID)
    set(KTXIMPORTER_TEST_DIR ".")
    set(BASISIMPORTER_TEST_DIR ".")
else()
    set(KTXIMPORTER_TEST_DIR ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/KtxImporter/Test)
    set(BASISIMPORTER_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(NOT MAGNUM_BASISIMPORTER_BUILD_STATIC)
    set(BASISIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:BasisImporter>)
    if(MAGNUM_WITH_STBIMAGEIMPORTER)
        set(STBIMAGEIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:StbImageImporter>)
    endif()
    if(MAGNUM_WITH_OPENEXRIMPORTER)
        set(OPENEXRIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:OpenExrImporter>)
    endif()
    if(MAGNUM_WITH_BCDECIMAGECONVERTER)
        set(BCDECIMAGECONVERTER_PLUGIN_FILENAME $<TARGET_FILE:BcDecImageConverter>)
    endif()
    if(MAGNUM_WITH_ETCDECIMAGECONVERTER)
        set(ETCDECIMAGECONVERTER_PLUGIN_FILENAME $<TARGET_FILE:EtcDecImageConverter>)
    endif()
endif()

# First replace ${} variables, then $<> generator expressions
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/configure.h
    INPUT ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)

corrade_add_test(BasisImporterTest BasisImporterTest.cpp
    LIBRARIES Magnum::Trade Magnum::DebugTools
    FILES
        invalid-cube-face-count.basis
        invalid-cube-face-size.basis
        rgb.basis
        rgb.ktx2
        rgbf.basis
        rgbf.ktx2
        rgb-pow2.basis
        rgb-pow2.ktx2
        rgb-linear.basis
        rgb-linear.ktx2
        rgb-linear-pow2.basis
        rgb-linear-pow2.ktx2
        rgb-noflip.basis
        rgb-noflip.ktx2
        rgb-noflip-pow2.ktx2
        rgba.basis
        rgba.ktx2
        rgbaf.basis
        rgbaf.ktx2
        rgba-pow2.basis
        rgba-pow2.ktx2
        rgba-uastc.basis
        rgba-uastc.ktx2
        rgba-2images-mips.basis
        rgba-3d.basis
        rgba-3d.ktx2
        rgba-3d-mips.basis
        rgba-3d-mips.ktx2
        rgba-array.basis
        rgba-array.ktx2
        rgba-array-mips.basis
        rgba-array-mips.ktx2
        rgba-cubemap.basis
        rgba-cubemap.ktx2
        rgba-cubemap-array.basis
        rgba-cubemap-array.ktx2
        rgba-noflip-array.ktx2
        rgba-video.basis
        rgba-video.ktx2
        rgba-video-uastc.basis
        rgba-video-uastc.ktx2
        rgb-63x27.png
        rgb-63x27.exr
        rgba-63x27.png
        rgba-63x27.exr
        rgba-63x27-slice1.png
        rgba-63x27-slice2.png
        rgba-31x13.png
        rgba-13x31.png
        rgba-15x6.png
        rgba-6x15.png
        rgba-27x27.png
        rgba-27x27-slice1.png
        rgba-27x27-slice2.png
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/KtxImporter/Test/2d-rgba.ktx2)
target_include_directories(BasisImporterTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
# The test needs BASISD_LIB_VERSION from basisu_transcoder.h
target_include_directories(BasisImporterTest SYSTEM PRIVATE ${BasisUniversalTranscoder_INCLUDE_DIR})
# BasisUniversalTranscoder_DEFINITIONS isn't exported by find_package, so we
# need to grab it from the target
get_target_property(BasisUniversalTranscoder_DEFINITIONS BasisUniversal::Transcoder
    INTERFACE_COMPILE_DEFINITIONS)
set_property(TARGET BasisImporterTest APPEND PROPERTY
    COMPILE_DEFINITIONS ${BasisUniversalTranscoder_DEFINITIONS})
if(MAGNUM_BASISIMPORTER_BUILD_STATIC)
    target_link_libraries(BasisImporterTest PRIVATE BasisImporter)
    if(Magnum_AnyImageImporter_FOUND)
        target_link_libraries(BasisImporterTest PRIVATE Magnum::AnyImageImporter)
    endif()
    if(MAGNUM_WITH_STBIMAGEIMPORTER)
        target_link_libraries(BasisImporterTest PRIVATE StbImageImporter)
    endif()
    if(MAGNUM_WITH_OPENEXRIMPORTER)
        target_link_libraries(BasisImporterTest PRIVATE OpenExrImporter)
    endif()
    if(MAGNUM_WITH_BCDECIMAGECONVERTER)
        target_link_libraries(BasisImporterTest PRIVATE BcDecImageConverter)
    endif()
    if(MAGNUM_WITH_ETCDECIMAGECONVERTER)
        target_link_libraries(BasisImporterTest PRIVATE EtcDecImageConverter)
    endif()
else()
    # So the plugins get properly built when building the test
    add_dependencies(BasisImporterTest BasisImporter)
    if(MAGNUM_WITH_STBIMAGEIMPORTER)
        add_dependencies(BasisImporterTest StbImageImporter)
    endif()
    if(MAGNUM_WITH_OPENEXRIMPORTER)
        add_dependencies(BasisImporterTest OpenExrImporter)
    endif()
    if(MAGNUM_WITH_BCDECIMAGECONVERTER)
        add_dependencies(BasisImporterTest BcDecImageConverter)
    endif()
    if(MAGNUM_WITH_ETCDECIMAGECONVERTER)
        add_dependencies(BasisImporterTest EtcDecImageConverter)
    endif()
endif()
if(CORRADE_BUILD_STATIC AND NOT MAGNUM_BASISIMPORTER_BUILD_STATIC)
    # CMake < 3.4 does this implicitly, but 3.4+ not anymore (see CMP0065).
    # That's generally okay, *except if* the build is static, the executable
    # uses a plugin manager and needs to share globals with the plugins (such
    # as output redirection and so on).
    set_target_properties(BasisImporterTest PROPERTIES ENABLE_EXPORTS ON)
endif()
