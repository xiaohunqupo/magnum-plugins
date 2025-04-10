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
set(CMAKE_FOLDER "MagnumPlugins/StanfordImporter/Test")

if(CORRADE_TARGET_EMSCRIPTEN OR CORRADE_TARGET_ANDROID)
    set(STANFORDIMPORTER_TEST_DIR ".")
else()
    set(STANFORDIMPORTER_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(NOT MAGNUM_STANFORDIMPORTER_BUILD_STATIC)
    set(STANFORDIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:StanfordImporter>)
    if(MAGNUM_WITH_ASSIMPIMPORTER)
        set(ASSIMPIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:AssimpImporter>)
    endif()
endif()

# First replace ${} variables, then $<> generator expressions
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/configure.h
    INPUT ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)

corrade_add_test(StanfordImporterTest StanfordImporterTest.cpp
    LIBRARIES Magnum::Trade
    FILES
        ascii.ply
        colors-not-same-type.ply
        colors-not-all.ply
        colors-not-tightly-packed.ply
        colors-unsupported-type.ply
        colors4-not-same-type.ply
        colors4-not-tightly-packed.ply
        custom-components.ply
        custom-components-be.ply
        custom-components-duplicate.ply
        crlf.ply
        empty.ply
        format-invalid.ply
        format-missing.ply
        format-too-late.ply
        format-unsupported.ply
        incomplete-face-specification.ply
        invalid-face-index-type.ply
        invalid-face-property.ply
        invalid-face-size-type.ply
        invalid-face-type.ply
        invalid-signature.ply
        invalid-vertex-property.ply
        invalid-vertex-type.ply
        normals-not-same-type.ply
        normals-not-all.ply
        normals-not-tightly-packed.ply
        normals-unsupported-type.ply
        objectid-unsupported-type.ply
        per-face-colors-be.ply
        per-face-normals-objectid.ply
        positions-colors-normals-texcoords-float-objectid-uint-indices-int-be.ply
        positions-colors-normals-texcoords-float-objectid-uint-indices-int.ply
        positions-colors4-normals-texcoords-float-indices-int-be-unaligned.ply
        positions-colors4-float-indices-int.ply
        positions-float-indices-uint.ply
        positions-char-colors4-ushort-texcoords-uchar-indices-short-be.ply
        positions-short-colors-uchar-texcoords-ushort-indices-char.ply
        positions-uchar-normals-char-objectid-short-indices-ushort.ply
        positions-ushort-normals-short-objectid-uchar-indices-uchar-be.ply
        positions-missing.ply
        positions-not-same-type.ply
        positions-not-all.ply
        positions-not-tightly-packed.ply
        positions-unsupported-type.ply
        texcoords-not-same-type.ply
        texcoords-not-all.ply
        texcoords-not-tightly-packed.ply
        texcoords-unsupported-type.ply
        triangle-fast-path-be.ply
        unexpected-property.ply
        unknown-element.ply
        unknown-line.ply
        unsupported-face-size.ply)
target_include_directories(StanfordImporterTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
if(MAGNUM_STANFORDIMPORTER_BUILD_STATIC)
    target_link_libraries(StanfordImporterTest PRIVATE StanfordImporter)
    if(MAGNUM_WITH_ASSIMPIMPORTER)
        target_link_libraries(StanfordImporterTest PRIVATE AssimpImporter)
    endif()
else()
    # So the plugins get properly built when building the test
    add_dependencies(StanfordImporterTest StanfordImporter)
    if(MAGNUM_WITH_ASSIMPIMPORTER)
        add_dependencies(StanfordImporterTest AssimpImporter)
    endif()
endif()
if(CORRADE_BUILD_STATIC AND NOT MAGNUM_STANFORDIMPORTER_BUILD_STATIC)
    # CMake < 3.4 does this implicitly, but 3.4+ not anymore (see CMP0065).
    # That's generally okay, *except if* the build is static, the executable
    # uses a plugin manager and needs to share globals with the plugins (such
    # as output redirection and so on).
    set_target_properties(StanfordImporterTest PROPERTIES ENABLE_EXPORTS ON)
endif()
