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
set(CMAKE_FOLDER "Magnum/OpenDdl")

if(MAGNUM_BUILD_STATIC)
    set(MAGNUM_OPENDDL_BUILD_STATIC 1)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

# Internal implementation that needs to be accessed by the test
add_library(MagnumOpenDdlObjects OBJECT
    Implementation/Parsers.cpp
    Implementation/Parsers.h)
target_include_directories(MagnumOpenDdlObjects PUBLIC
    $<TARGET_PROPERTY:Magnum::Magnum,INTERFACE_INCLUDE_DIRECTORIES>
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src)
if(NOT MAGNUM_BUILD_STATIC)
    target_compile_definitions(MagnumOpenDdlObjects PRIVATE "MagnumOpenDdlsObjects_EXPORTS")
endif()
if(NOT MAGNUM_BUILD_STATIC OR MAGNUM_BUILD_STATIC_PIC)
    set_target_properties(MagnumOpenDdlObjects PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

set(MagnumOpenDdl_SRCS
    OpenDdl.cpp)

set(MagnumOpenDdl_HEADERS
    Document.h
    OpenDdl.h
    Property.h
    Structure.h
    Type.h
    Validation.h
    visibility.h)

# OpenDDL parser library
add_library(MagnumOpenDdl ${SHARED_OR_STATIC}
    ${MagnumOpenDdl_SRCS}
    ${MagnumOpenDdl_HEADERS}
    $<TARGET_OBJECTS:MagnumOpenDdlObjects>)
target_include_directories(MagnumOpenDdl PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src)
target_link_libraries(MagnumOpenDdl PUBLIC Magnum::Magnum)
if(NOT MAGNUM_BUILD_STATIC)
    set_target_properties(MagnumOpenDdl PROPERTIES VERSION ${MAGNUMPLUGINS_LIBRARY_VERSION} SOVERSION ${MAGNUMPLUGINS_LIBRARY_SOVERSION})
elseif(MAGNUM_BUILD_STATIC AND MAGNUM_BUILD_STATIC_PIC)
    set_target_properties(MagnumOpenDdl PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
set_target_properties(MagnumOpenDdl PROPERTIES DEBUG_POSTFIX "-d")

install(TARGETS MagnumOpenDdl
    RUNTIME DESTINATION ${MAGNUM_BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${MAGNUM_LIBRARY_INSTALL_DIR}
    ARCHIVE DESTINATION ${MAGNUM_LIBRARY_INSTALL_DIR})
install(FILES ${MagnumOpenDdl_HEADERS} DESTINATION ${MAGNUM_INCLUDE_INSTALL_DIR}/OpenDdl)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/configure.h DESTINATION ${MAGNUM_INCLUDE_INSTALL_DIR}/OpenDdl)

if(MAGNUM_BUILD_TESTS)
    add_subdirectory(Test ${EXCLUDE_FROM_ALL_IF_TEST_TARGET})
endif()

# MagnumPlugins OpenDdl target alias for superprojects
add_library(MagnumPlugins::OpenDdl ALIAS MagnumOpenDdl)
