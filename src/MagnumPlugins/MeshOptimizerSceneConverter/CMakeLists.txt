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

find_package(Magnum REQUIRED MeshTools Trade)

if(NOT TARGET meshoptimizer)
    find_package(meshoptimizer REQUIRED CONFIG)
else()
    add_library(meshoptimizer::meshoptimizer ALIAS meshoptimizer)
endif()

if(MAGNUM_BUILD_PLUGINS_STATIC AND NOT DEFINED MAGNUM_MESHOPTIMIZERSCENECONVERTER_BUILD_STATIC)
    set(MAGNUM_MESHOPTIMIZERSCENECONVERTER_BUILD_STATIC 1)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

# MeshOptimizerSceneConverter plugin
add_plugin(MeshOptimizerSceneConverter
    sceneconverters
    "${MAGNUM_PLUGINS_SCENECONVERTER_DEBUG_BINARY_INSTALL_DIR};${MAGNUM_PLUGINS_SCENECONVERTER_DEBUG_LIBRARY_INSTALL_DIR}"
    "${MAGNUM_PLUGINS_SCENECONVERTER_RELEASE_BINARY_INSTALL_DIR};${MAGNUM_PLUGINS_SCENECONVERTER_RELEASE_LIBRARY_INSTALL_DIR}"
    MeshOptimizerSceneConverter.conf
    MeshOptimizerSceneConverter.cpp
    MeshOptimizerSceneConverter.h)
if(MAGNUM_MESHOPTIMIZERSCENECONVERTER_BUILD_STATIC AND MAGNUM_BUILD_STATIC_PIC)
    set_target_properties(MeshOptimizerSceneConverter PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
target_include_directories(MeshOptimizerSceneConverter PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src)
target_link_libraries(MeshOptimizerSceneConverter PUBLIC
    Magnum::MeshTools
    Magnum::Trade
    meshoptimizer::meshoptimizer)

install(FILES MeshOptimizerSceneConverter.h ${CMAKE_CURRENT_BINARY_DIR}/configure.h
    DESTINATION ${MAGNUM_PLUGINS_INCLUDE_INSTALL_DIR}/MeshOptimizerSceneConverter)

# Automatic static plugin import
if(MAGNUM_MESHOPTIMIZERSCENECONVERTER_BUILD_STATIC)
    install(FILES importStaticPlugin.cpp DESTINATION ${MAGNUM_PLUGINS_INCLUDE_INSTALL_DIR}/MeshOptimizerSceneConverter)
    target_sources(MeshOptimizerSceneConverter INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/importStaticPlugin.cpp)
endif()

if(MAGNUM_BUILD_TESTS)
    add_subdirectory(Test ${EXCLUDE_FROM_ALL_IF_TEST_TARGET})
endif()

# MagnumPlugins MeshOptimizerSceneConverter target alias for superprojects
add_library(MagnumPlugins::MeshOptimizerSceneConverter ALIAS MeshOptimizerSceneConverter)
