#
#   This file is part of Magnum.
#
#   Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
#               2020, 2021, 2022, 2023, 2024, 2025
#             Vladimír Vondruš <mosra@centrum.cz>
#   Copyright © 2019 Jonathan Hale <squareys@googlemail.com>
#   Copyright © 2021, 2022, 2024 Pablo Escobar <mail@rvrs.in>
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
set(CMAKE_FOLDER "MagnumPlugins/BasisImageConverter/Test")

find_package(Magnum REQUIRED
    COMPONENTS DebugTools
    OPTIONAL_COMPONENTS AnyImageImporter)

# See BasisImageConverter.h for details -- the plugin itself can't be linked to
# pthread, the app has to be instead. As of 2022-01-28, the CI nondeprecated
# (w/o asserts, Release) build started mysteriously failing unless
# THREADS_PREFER_PTHREAD_FLAG is set, with -lpthread doing nothing and the test
# then aborting due to pthread not being linked. The regular build works
# properly, -lpthread linking everything. Didn't investigate further (could it
# be due to the removed assertions which kinda implied pthread being needed or
# some such?), might've been caused by some glibc update(?) that dropped some
# old aspects of -lpthread which are assumed to not be used anymore AND YET
# CMAKE STILL DEFAULTS TO THAT.
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# FindBasisUniversal, when building from source with BASIS_UNIVERSAL_DIR,
# builds with OpenCL support if it finds the package. Pass OpenCL_FOUND to
# configure.h so the test knows if OpenCL is supported.
find_package(OpenCL QUIET)

if(CORRADE_TARGET_EMSCRIPTEN OR CORRADE_TARGET_ANDROID)
    set(BASISIMAGECONVERTER_TEST_OUTPUT_DIR "write")
else()
    set(BASISIMAGECONVERTER_TEST_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
endif()

if(MAGNUM_WITH_BASISIMPORTER)
    if(CORRADE_TARGET_EMSCRIPTEN OR CORRADE_TARGET_ANDROID)
        set(BASISIMPORTER_TEST_DIR ".")
    else()
        set(BASISIMPORTER_TEST_DIR ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/BasisImporter/Test)
    endif()
endif()

if(NOT MAGNUM_BASISIMAGECONVERTER_BUILD_STATIC)
    set(BASISIMAGECONVERTER_PLUGIN_FILENAME $<TARGET_FILE:BasisImageConverter>)
    if(MAGNUM_WITH_BASISIMPORTER)
        set(BASISIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:BasisImporter>)
    endif()
    if(MAGNUM_WITH_STBIMAGEIMPORTER)
        set(STBIMAGEIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:StbImageImporter>)
    endif()
    if(MAGNUM_WITH_OPENEXRIMPORTER)
        set(OPENEXRIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:OpenExrImporter>)
    endif()
endif()

# On Mac OpenCL can be found but the runtime implementation could fail, listing
# zero usable devices. The test assumes that OpenCL being found means encoding
# with OpenCL succeeds, let it know to expect failure in this case.
if(CORRADE_TARGET_APPLE AND OpenCL_LIBRARY MATCHES "System/Library/Frameworks/OpenCL\\.framework")
    set(_BASISIMAGECONVERTER_EXPECT_OPENCL_FRAMEWORK_FAILURE ON)
endif()

# First replace ${} variables, then $<> generator expressions
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/configure.h
    INPUT ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)

corrade_add_test(BasisImageConverterTest BasisImageConverterTest.cpp
    LIBRARIES
        # See BasisImageConverter.h for details -- the plugin itself can't be
        # linked to pthread, the app has to be instead
        Magnum::Trade Magnum::DebugTools
        Threads::Threads
    FILES
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/BasisImporter/Test/rgb-63x27.png
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/BasisImporter/Test/rgba-27x27.png
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/BasisImporter/Test/rgba-63x27.png
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/BasisImporter/Test/rgba-31x13.png
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/BasisImporter/Test/rgba-15x6.png
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/BasisImporter/Test/rgba-63x27.exr
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/BasisImporter/Test/rgba-31x13.exr
        ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/BasisImporter/Test/rgba-15x6.exr)
target_include_directories(BasisImageConverterTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
# The test needs BASISU_LIB_VERSION from basisu_comp.h
target_include_directories(BasisImageConverterTest SYSTEM PRIVATE ${BasisUniversalEncoder_INCLUDE_DIR})
# BasisUniversalEncoder_DEFINITIONS isn't exported by find_package, so we need
# to grab it from the target
get_target_property(BasisUniversalEncoder_DEFINITIONS BasisUniversal::Encoder
    INTERFACE_COMPILE_DEFINITIONS)
set_property(TARGET BasisImageConverterTest APPEND PROPERTY
    COMPILE_DEFINITIONS ${BasisUniversalEncoder_DEFINITIONS})
if(MAGNUM_BASISIMAGECONVERTER_BUILD_STATIC)
    target_link_libraries(BasisImageConverterTest PRIVATE BasisImageConverter)
    if(Magnum_AnyImageImporter_FOUND)
        target_link_libraries(BasisImageConverterTest PRIVATE Magnum::AnyImageImporter)
    endif()
    if(MAGNUM_WITH_BASISIMPORTER)
        target_link_libraries(BasisImageConverterTest PRIVATE BasisImporter)
    endif()
    if(MAGNUM_WITH_STBIMAGEIMPORTER)
        target_link_libraries(BasisImageConverterTest PRIVATE StbImageImporter)
    endif()
    if(MAGNUM_WITH_OPENEXRIMPORTER)
        target_link_libraries(BasisImageConverterTest PRIVATE OpenExrImporter)
    endif()
else()
    # So the plugins get properly built when building the test
    add_dependencies(BasisImageConverterTest BasisImageConverter)
    if(MAGNUM_WITH_BASISIMPORTER)
        add_dependencies(BasisImageConverterTest BasisImporter)
    endif()
    if(MAGNUM_WITH_STBIMAGEIMPORTER)
        add_dependencies(BasisImageConverterTest StbImageImporter)
    endif()
    if(MAGNUM_WITH_OPENEXRIMPORTER)
        add_dependencies(BasisImageConverterTest OpenExrImporter)
    endif()
endif()
if(CORRADE_BUILD_STATIC AND NOT MAGNUM_BASISIMAGECONVERTER_BUILD_STATIC)
    # CMake < 3.4 does this implicitly, but 3.4+ not anymore (see CMP0065).
    # That's generally okay, *except if* the build is static, the executable
    # uses a plugin manager and needs to share globals with the plugins (such
    # as output redirection and so on).
    set_target_properties(BasisImageConverterTest PROPERTIES ENABLE_EXPORTS ON)
endif()
