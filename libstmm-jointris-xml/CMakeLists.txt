# File: libstmm-jointris-xml/CMakeLists.txt

#  Copyright © 2019-2020  Stefano Marsili, <stemars@gmx.ch>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public
#  License along with this program; if not, see <http://www.gnu.org/licenses/>

cmake_minimum_required(VERSION 3.0)

project(stmm-jointris-xml CXX)

set(LIBRARY_OUTPUT_DIRECTORY "build")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../share/cmake")

include(CommonUtil)

CheckBinaryNotSourceTree()
CheckBuildType()
DefineSharedLibOption()
DefineCommonOptions()
DefineCommonCompileOptions("c++14")

# Headers dir
set(STMMI_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/include")
set(STMMI_HEADERS_DIR  "${STMMI_INCLUDE_DIR}/stmm-jointris-xml")
# Header files
set(STMMI_HEADERS
        "${STMMI_HEADERS_DIR}/stmm-jointris-xml-config.h"
        "${STMMI_HEADERS_DIR}/xmlblockevent.h"
        "${STMMI_HEADERS_DIR}/xmlfeederevent.h"
        "${STMMI_HEADERS_DIR}/xmlrowpusherevent.h"
        "${STMMI_HEADERS_DIR}/xmlrowremoverevent.h"
        )
#
# Sources dir
set(STMMI_SOURCES_DIR  "${PROJECT_SOURCE_DIR}/src")
# Source files (and headers only used for building)
set(STMMI_SOURCES
        "${STMMI_SOURCES_DIR}/xmlblockevent.cc"
        "${STMMI_SOURCES_DIR}/xmlfeederevent.cc"
        "${STMMI_SOURCES_DIR}/xmlrowpusherevent.cc"
        "${STMMI_SOURCES_DIR}/xmlrowremoverevent.cc"
        )

# Data dir
set(STMMI_DATA_DIR  "${PROJECT_SOURCE_DIR}/data")
# Data files
set(STMMI_DATA_FILES
        "${STMMI_DATA_DIR}/themes/jointris/simple_j.thm/theme.xml"
        "${STMMI_DATA_DIR}/themes/jointris/simple_j.thm/thumbnail.png"
        "${STMMI_DATA_DIR}/themes/jointris/simple_j.thm/sounds/freezed.mp3"
        "${STMMI_DATA_DIR}/themes/jointris/simple_j.thm/sounds/fused.mp3"
        "${STMMI_DATA_DIR}/themes/jointris/simple_j.thm/sounds/explosion.mp3" # overrides stmm-games file
        )

# Define library
add_library(stmm-jointris-xml ${STMMI_SOURCES} "${PROJECT_BINARY_DIR}/stmm-jointris-xml-config.cc")

include("stmm-jointris-xml-defs.cmake")

target_include_directories(stmm-jointris-xml        PUBLIC ${STMMJOINTRISXML_EXTRA_INCLUDE_DIRS})
target_include_directories(stmm-jointris-xml SYSTEM PUBLIC ${STMMJOINTRISXML_EXTRA_INCLUDE_SDIRS})
target_include_directories(stmm-jointris-xml BEFORE PUBLIC
  $<BUILD_INTERFACE:${STMMI_HEADERS_DIR}>
  $<INSTALL_INTERFACE:include/stmm-jointris-xml>  # <prefix>/include/stmm-jointris-xml
)

target_link_libraries(stmm-jointris-xml ${STMMJOINTRISXML_EXTRA_LIBRARIES})

set_target_properties(stmm-jointris-xml PROPERTIES ${CMAKE_BUILD_TYPE}_POSTFIX "")
set_target_properties(stmm-jointris-xml PROPERTIES
        VERSION ${STMM_JOINTRIS_XML_VERSION}
        SOVERSION ${STMM_JOINTRIS_XML_MAJOR_VERSION}
#        OUTPUT_NAME "stmm-jointris-xml-${STMM_JOINTRIS_XML_VERSION}"
        )

DefineTargetPublicCompileOptions(stmm-jointris-xml)

# Set version and dirs for stmm-jointris-xml-config.cc.in
set(STMMI_PKG_VERSION ${STMM_JOINTRIS_XML_VERSION})
include(GNUInstallDirs)
set(STMMI_PKG_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/stmm-games")

# Create config file for library
configure_file("${PROJECT_SOURCE_DIR}/src/stmm-jointris-xml-config.cc.in"
               "${PROJECT_BINARY_DIR}/stmm-jointris-xml-config.cc" @ONLY)

# Set install dir for pkg-config file
set(STMMI_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
# Create pkg-config file for the library
configure_file("${PROJECT_SOURCE_DIR}/stmm-jointris-xml.pc.in"
               "${PROJECT_BINARY_DIR}/stmm-jointris-xml.pc" @ONLY)

if ($ENV{STMM_CMAKE_COMMENTS})
message(STATUS "")
message(STATUS "stmm-jointris-xml was configured with the following options:")
message(STATUS " STMMI_HEADERS:                       ${STMMI_HEADERS}")
message(STATUS " STMMI_SOURCES:                       ${STMMI_SOURCES}")
message(STATUS " STMMJOINTRISXML_EXTRA_INCLUDE_DIRS:  ${STMMJOINTRISXML_EXTRA_INCLUDE_DIRS}")
message(STATUS " STMMJOINTRISXML_EXTRA_INCLUDE_SDIRS: ${STMMJOINTRISXML_EXTRA_INCLUDE_SDIRS}")
message(STATUS " STMMJOINTRISXML_EXTRA_LIBRARIES:     ${STMMJOINTRISXML_EXTRA_LIBRARIES}")
message(STATUS " CMAKE_BUILD_TYPE:                    ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER_ID:               ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " CMAKE_CXX_COMPILER_VERSION:          ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " CMAKE_CXX_FLAGS:                     ${CMAKE_CXX_FLAGS}")
message(STATUS " install prefix:                      ${STMMI_INSTALL_PREFIX}")
message(STATUS " BUILD_DOCS:                          ${BUILD_DOCS}")
message(STATUS " ENV{STMM_DEBIAN_PACKAGING}:          $ENV{STMM_DEBIAN_PACKAGING}")
message(STATUS " BUILD_TESTING:                       ${BUILD_TESTING}")
message(STATUS " STMMI_PKG_DATA_DIR:                  ${STMMI_PKG_DATA_DIR}")
endif()

# Documentation
add_subdirectory(doc)
if (BUILD_DOCS)
    # build automatically, otherwise call `make doc`
    add_dependencies(stmm-jointris-xml doc)
endif()

# Testing
enable_testing()
add_subdirectory(test)

install(TARGETS stmm-jointris-xml LIBRARY DESTINATION "lib"  ARCHIVE DESTINATION "lib")

install(FILES ${STMMI_HEADERS} DESTINATION "include/stmm-jointris-xml")

install(FILES  "${PROJECT_BINARY_DIR}/stmm-jointris-xml.pc"   DESTINATION "lib/pkgconfig")

foreach (STMMI_DATA_CUR_FILE  ${STMMI_DATA_FILES})
    file(RELATIVE_PATH  STMMI_DATA_CUR_REL_FILE  "${STMMI_DATA_DIR}"  "${STMMI_DATA_CUR_FILE}")
    get_filename_component(STMMI_DATA_CUR_DIR "${STMMI_PKG_DATA_DIR}/${STMMI_DATA_CUR_REL_FILE}" DIRECTORY)
    #message(STATUS "STMMI_DATA_CUR_FILE         ${STMMI_DATA_CUR_FILE}")
    #message(STATUS "STMMI_DATA_CUR_REL_FILE     ${STMMI_DATA_CUR_REL_FILE}")
    #message(STATUS "STMMI_DATA_CUR_DIR          ${STMMI_DATA_CUR_DIR}")
    install(FILES ${STMMI_DATA_CUR_FILE}  DESTINATION   "${STMMI_DATA_CUR_DIR}")
endforeach()
