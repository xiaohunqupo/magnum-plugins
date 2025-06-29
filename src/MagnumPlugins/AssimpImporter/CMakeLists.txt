#
#   This file is part of Magnum.
#
#   Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
#               2020, 2021, 2022, 2023, 2024, 2025
#             Vladimír Vondruš <mosra@centrum.cz>
#   Copyright © 2017 Jonathan Hale <squareys@googlemail.com>
#   Copyright © 2021, 2022 Pablo Escobar <mail@rvrs.in>
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

find_package(Magnum REQUIRED Trade AnyImageImporter)
find_package(Assimp REQUIRED)

if(MAGNUM_BUILD_PLUGINS_STATIC AND NOT DEFINED MAGNUM_ASSIMPIMPORTER_BUILD_STATIC)
    set(MAGNUM_ASSIMPIMPORTER_BUILD_STATIC 1)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

# Use try_compile() to figure out a concrete Assimp insanity level, as most
# relevant bugfixes are either not in any stable version yet or the version
# info is broken. Unfortunately, because try_compile() is run at configure
# time, LINK_LIBRARIES only works for imported targets as targets defined
# *inside* the project as are not built at that point yet. So when Assimp is a
# CMake subproject, it'll complain that Assimp::Assimp doesn't exist. To work
# around that issue, we build a static library (to avoid linking to anything)
# and supply INCLUDE_DIRECTORIES. Static library is only available since CMake
# 3.6, but eh, we can't have everything.
if(TARGET assimp) # Only defined if we have a CMake subproject
    get_target_property(_ASSIMP_INCLUDE_DIRECTORIES assimp SOURCE_DIR)
    get_target_property(_ASSIMP_BINARY_INCLUDE_DIRECTORIES assimp BINARY_DIR)
    get_filename_component(_ASSIMP_INCLUDE_DIRECTORIES ${_ASSIMP_INCLUDE_DIRECTORIES} DIRECTORY)
    get_filename_component(_ASSIMP_BINARY_INCLUDE_DIRECTORIES ${_ASSIMP_BINARY_INCLUDE_DIRECTORIES} DIRECTORY)
    set(_ASSIMP_TRY_COMPILE_LINK_OR_INCLUDE CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${_ASSIMP_INCLUDE_DIRECTORIES}/include\;${_ASSIMP_BINARY_INCLUDE_DIRECTORIES}/include")
    if(CMAKE_VERSION VERSION_LESS 3.6)
        message(FATAL_ERROR "Sorry, using Assimp as a CMake submodule requires CMake 3.6 in order to check for presence of various bugs. To continue, either upgrade CMake or use an external build of Assimp.")
    endif()
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
else()
    set(_ASSIMP_TRY_COMPILE_LINK_OR_INCLUDE LINK_LIBRARIES Assimp::Assimp)
endif()

# In addition to above, use a static library for try_compile() also to avoid
# annoying debug/release build and _ITERATOR_DEBUG_LEVEL mismatches on Windows
# (as they're only checked at link time). Without that, the user would need to
# make sure to set CMAKE_TRY_COMPILE_CONFIGURATION based on what Assimp
# libraries are found.
if(CORRADE_TARGET_WINDOWS)
    if(CMAKE_VERSION VERSION_LESS 3.6)
        message(FATAL_ERROR "Sorry, using Assimp on Windows requires CMake 3.6 in order to check for presence of various bugs. To continue, either upgrade CMake or use an external build of Assimp.")
    endif()
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
endif()

# Newer versions of Assimp add C compile features that end up being propagated
# to installed CMake targets:
# https://github.com/assimp/assimp/commit/799384f2b85b8d3ee9c5b9e18bbe532b4dc7c63c
#
# This can fail in try_compile because it uses the file extension to determine
# the language and then doesn't look for a C compiler for the required feature.
# CMAKE_PROJECT_[PROJECT]_INCLUDE allows us to include a script after any
# project() calls in try_compile, which then injects a call to
# enable_language(C). CMAKE_TRY_COMPILE is the name of the project in the
# temporary CMakeList.txt created by try_compile:
# https://github.com/Kitware/CMake/blob/v3.4.0/Source/cmCoreTryCompile.cxx#L313
#
# TODO: drop this and use a STATIC_LIBRARY when we can rely on CMake 3.6
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/checkAssimpVersionLanguageOverride.cmake "enable_language(C)")
# For some reason, this also needs C to be enabled in this scope
enable_language(C)

# Go through all versions of interest and pick the highest one that compiles
foreach(_version 20240717 20220502 20210102 20201123 20200225 20191122 20190915 20160716 20151031)
    try_compile(_works ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/checkAssimpVersion.cpp
        ${_ASSIMP_TRY_COMPILE_LINK_OR_INCLUDE}
        COMPILE_DEFINITIONS
            -DCHECK_VERSION=${_version}
            # I thought I could make this use C++11 by linking to
            # Corrade::Utility, but for some reason the effect of the
            # CORRADE_CXX_STANDARD property doesn't get passed through. So I
            # have to use an internal variable for that instead.
            ${CORRADE_CXX11_STANDARD_FLAG}
        CMAKE_FLAGS
            -DCMAKE_PROJECT_CMAKE_TRY_COMPILE_INCLUDE=${CMAKE_CURRENT_BINARY_DIR}/checkAssimpVersionLanguageOverride.cmake
        OUTPUT_VARIABLE _version_output)
    if(_works)
        set(ASSIMP_VERSION ${_version})
        break()
    endif()
endforeach()
if(NOT ASSIMP_VERSION)
    message(SEND_ERROR "Could not detect Assimp version. Maybe even older than 3.2? ${_version_output}")
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configureInternal.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configureInternal.h)

# On versions less than 5.4.3 (which is checked with 20240717 above) the
# UsdImporter alias gets commented out
if(ASSIMP_VERSION LESS 20240717)
    set(ASSIMP_USD_SUPPORT "#")
else()
    set(ASSIMP_USD_SUPPORT "")
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/AssimpImporter.conf.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/AssimpImporter.conf)

# AssimpImporter plugin
add_plugin(AssimpImporter
    importers
    "${MAGNUM_PLUGINS_IMPORTER_DEBUG_BINARY_INSTALL_DIR};${MAGNUM_PLUGINS_IMPORTER_DEBUG_LIBRARY_INSTALL_DIR}"
    "${MAGNUM_PLUGINS_IMPORTER_RELEASE_BINARY_INSTALL_DIR};${MAGNUM_PLUGINS_IMPORTER_RELEASE_LIBRARY_INSTALL_DIR}"
    ${CMAKE_CURRENT_BINARY_DIR}/AssimpImporter.conf
    AssimpImporter.cpp
    AssimpImporter.h)
target_include_directories(AssimpImporter PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
if(MAGNUM_ASSIMPIMPORTER_BUILD_STATIC AND MAGNUM_BUILD_STATIC_PIC)
    set_target_properties(AssimpImporter PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
target_include_directories(AssimpImporter PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src)
target_link_libraries(AssimpImporter PUBLIC Magnum::Trade Assimp::Assimp)
if(CORRADE_TARGET_WINDOWS)
    target_link_libraries(AssimpImporter PUBLIC Magnum::AnyImageImporter)
elseif(MAGNUM_ASSIMPIMPORTER_BUILD_STATIC)
    target_link_libraries(AssimpImporter INTERFACE Magnum::AnyImageImporter)
endif()

install(FILES AssimpImporter.h ${CMAKE_CURRENT_BINARY_DIR}/configure.h
    DESTINATION ${MAGNUM_PLUGINS_INCLUDE_INSTALL_DIR}/AssimpImporter)

# Automatic static plugin import
if(MAGNUM_ASSIMPIMPORTER_BUILD_STATIC)
    install(FILES importStaticPlugin.cpp DESTINATION ${MAGNUM_PLUGINS_INCLUDE_INSTALL_DIR}/AssimpImporter)
    target_sources(AssimpImporter INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/importStaticPlugin.cpp)
endif()

if(MAGNUM_BUILD_TESTS)
    add_subdirectory(Test ${EXCLUDE_FROM_ALL_IF_TEST_TARGET})
endif()

# MagnumPlugins AssimpImporter target alias for superprojects
add_library(MagnumPlugins::AssimpImporter ALIAS AssimpImporter)
