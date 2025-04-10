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

find_package(Magnum REQUIRED ShaderTools)
find_package(Glslang REQUIRED)

if(MAGNUM_BUILD_PLUGINS_STATIC AND NOT DEFINED MAGNUM_GLSLANGSHADERCONVERTER_BUILD_STATIC)
    set(MAGNUM_GLSLANGSHADERCONVERTER_BUILD_STATIC 1)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

# Of course it's always the shittiest but somehow ireplaceable libraries that
# opt into requiring C++17 and up even for their users, instead of just their
# own compilation, while meshoptimizer, libjpeg etc. still works with C++03. I
# am not interested in enabling C++17 always because of old compiler support
# and because of compile times, so instead I'm detecting version 13.1.0+ that
# contains the following ABSOLUTELY UTTERLY USELESS change that puts a
# std::variant member in a public API, getting absolutely no advantage
# whatsoever because the type index in there is still making that field two
# pointers large due to padding, or possibly worse, depending on how bad the
# STL is:
#   https://github.com/KhronosGroup/glslang/pull/3328
# The compilation *fails* if glslang doesn't require C++17, since the check is
# simpler that way. See the source file for details.
try_compile(_glslang_requires_cpp17 ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/checkGlslangRequiresCpp17.cpp
    LINK_LIBRARIES Glslang::Glslang)

# GlslangShaderConverter plugin
add_plugin(GlslangShaderConverter
    shaderconverters
    "${MAGNUM_PLUGINS_SHADERCONVERTER_DEBUG_BINARY_INSTALL_DIR};${MAGNUM_PLUGINS_SHADERCONVERTER_DEBUG_LIBRARY_INSTALL_DIR}"
    "${MAGNUM_PLUGINS_SHADERCONVERTER_RELEASE_BINARY_INSTALL_DIR};${MAGNUM_PLUGINS_SHADERCONVERTER_RELEASE_LIBRARY_INSTALL_DIR}"
    GlslangConverter.conf
    GlslangConverter.cpp
    GlslangConverter.h)
if(MAGNUM_GLSLANGSHADERCONVERTER_BUILD_STATIC AND MAGNUM_BUILD_STATIC_PIC)
    set_target_properties(GlslangShaderConverter PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
if(_glslang_requires_cpp17)
    set_target_properties(GlslangShaderConverter PROPERTIES CORRADE_CXX_STANDARD 17)
else()

endif()
target_include_directories(GlslangShaderConverter PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src)
target_link_libraries(GlslangShaderConverter PUBLIC
    Magnum::ShaderTools
    Glslang::Glslang)

install(FILES GlslangConverter.h ${CMAKE_CURRENT_BINARY_DIR}/configure.h
    DESTINATION ${MAGNUM_PLUGINS_INCLUDE_INSTALL_DIR}/GlslangShaderConverter)

# Automatic static plugin import
if(MAGNUM_GLSLANGSHADERCONVERTER_BUILD_STATIC)
    install(FILES importStaticPlugin.cpp DESTINATION ${MAGNUM_PLUGINS_INCLUDE_INSTALL_DIR}/GlslangShaderConverter)
    target_sources(GlslangShaderConverter INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/importStaticPlugin.cpp)
endif()

if(MAGNUM_BUILD_TESTS)
    add_subdirectory(Test ${EXCLUDE_FROM_ALL_IF_TEST_TARGET})
endif()

# MagnumPlugins GlslangShaderConverter target alias for superprojects
add_library(MagnumPlugins::GlslangShaderConverter ALIAS GlslangShaderConverter)
