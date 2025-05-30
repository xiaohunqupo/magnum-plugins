#
#   This file is part of Magnum.
#
#   Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
#               2020, 2021, 2022, 2023, 2024, 2025
#             Vladimír Vondruš <mosra@centrum.cz>
#   Copyright © 2016 Alice Margatroid <loveoverwhelming@gmail.com>
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
find_package(DevIL REQUIRED)

if(MAGNUM_BUILD_PLUGINS_STATIC AND NOT DEFINED MAGNUM_DEVILIMAGEIMPORTER_BUILD_STATIC)
    set(MAGNUM_DEVILIMAGEIMPORTER_BUILD_STATIC 1)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

# DevIlImageImporter plugin
add_plugin(DevIlImageImporter
    importers
    "${MAGNUM_PLUGINS_IMPORTER_DEBUG_BINARY_INSTALL_DIR};${MAGNUM_PLUGINS_IMPORTER_DEBUG_LIBRARY_INSTALL_DIR}"
    "${MAGNUM_PLUGINS_IMPORTER_RELEASE_BINARY_INSTALL_DIR};${MAGNUM_PLUGINS_IMPORTER_RELEASE_LIBRARY_INSTALL_DIR}"
    DevIlImageImporter.conf
    DevIlImageImporter.cpp
    DevIlImageImporter.h)
if(MAGNUM_DEVILIMAGEIMPORTER_BUILD_STATIC AND MAGNUM_BUILD_STATIC_PIC)
    set_target_properties(DevIlImageImporter PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
# IL_INCLUDE_DIR in the upstream FindDevIL.cmake is "The path to the directory
# that contains il.h", but ilu.h includes IL/il.h, so IL_INCLUDE_DIR is
# basically useless and we need to go one level up. However, just saying
# ${IL_INCLUDE_DIR}/.. here wouldn't work, because this directory is included
# as system and saying `-isystem /usr/include` hits
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=70129 on GCC 6+. Because of
# that, I had to fork FindDevIL.cmake (see modules/ directory) to provide
# IL_INCLUDE_DIR without the IL/ subdirectory.
target_include_directories(DevIlImageImporter SYSTEM PRIVATE ${IL_INCLUDE_DIR})
target_include_directories(DevIlImageImporter PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src)
target_link_libraries(DevIlImageImporter PUBLIC
    Magnum::Trade
    ${IL_LIBRARIES})

install(FILES DevIlImageImporter.h ${CMAKE_CURRENT_BINARY_DIR}/configure.h
    DESTINATION ${MAGNUM_PLUGINS_INCLUDE_INSTALL_DIR}/DevIlImageImporter)

# Automatic static plugin import
if(MAGNUM_DEVILIMAGEIMPORTER_BUILD_STATIC)
    install(FILES importStaticPlugin.cpp DESTINATION ${MAGNUM_PLUGINS_INCLUDE_INSTALL_DIR}/DevIlImageImporter)
    target_sources(DevIlImageImporter INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/importStaticPlugin.cpp)
endif()

if(MAGNUM_BUILD_TESTS)
    add_subdirectory(Test ${EXCLUDE_FROM_ALL_IF_TEST_TARGET})
endif()

# MagnumPlugins DevIlImageImporter target alias for superprojects
add_library(MagnumPlugins::DevIlImageImporter ALIAS DevIlImageImporter)
