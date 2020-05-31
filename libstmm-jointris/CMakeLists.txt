# File: libstmm-jointris/CMakeLists.txt

#  Copyright © 2019  Stefano Marsili, <stemars@gmx.ch>
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

project(stmm-jointris CXX)

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
set(STMMI_HEADERS_DIR  "${STMMI_INCLUDE_DIR}/stmm-jointris")
# Header files
set(STMMI_HEADERS
        "${STMMI_HEADERS_DIR}/blockevent.h"
        "${STMMI_HEADERS_DIR}/feederevent.h"
        "${STMMI_HEADERS_DIR}/rowpusherevent.h"
        "${STMMI_HEADERS_DIR}/rowremoverevent.h"
        "${STMMI_HEADERS_DIR}/stmm-jointris-config.h"
        )
#
# Sources dir
set(STMMI_SOURCES_DIR  "${PROJECT_SOURCE_DIR}/src")
# Source files (and headers only used for building)
set(STMMI_SOURCES
        "${STMMI_SOURCES_DIR}/blockevent.cc"
        "${STMMI_SOURCES_DIR}/feederevent.cc"
        "${STMMI_SOURCES_DIR}/rowpusherevent.cc"
        "${STMMI_SOURCES_DIR}/rowremoverevent.cc"
        )

# Define library
add_library(stmm-jointris ${STMMI_SOURCES} "${PROJECT_BINARY_DIR}/stmm-jointris-config.cc")

include("stmm-jointris-defs.cmake")

target_include_directories(stmm-jointris        PUBLIC ${STMMJOINTRIS_EXTRA_INCLUDE_DIRS})
target_include_directories(stmm-jointris SYSTEM PUBLIC ${STMMJOINTRIS_EXTRA_INCLUDE_SDIRS})
target_include_directories(stmm-jointris BEFORE PUBLIC
  $<BUILD_INTERFACE:${STMMI_HEADERS_DIR}>
  $<INSTALL_INTERFACE:include/stmm-jointris>  # <prefix>/include/stmm-jointris
)
 
target_link_libraries(stmm-jointris ${STMMJOINTRIS_EXTRA_LIBRARIES})

set_target_properties(stmm-jointris PROPERTIES ${CMAKE_BUILD_TYPE}_POSTFIX "")
set_target_properties(stmm-jointris PROPERTIES
        VERSION ${STMM_JOINTRIS_VERSION}
        SOVERSION ${STMM_JOINTRIS_MAJOR_VERSION}
#        OUTPUT_NAME "stmm-jointris-${STMM_JOINTRIS_VERSION}"
        )

DefineTargetPublicCompileOptions(stmm-jointris)

include(GNUInstallDirs)
set(STMMI_PKG_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/stmm-games")
# Set version for stmm-jointris-config.cc.in
set(STMMI_PKG_VERSION "${STMM_JOINTRIS_VERSION}")

# Create config file for library
configure_file("${PROJECT_SOURCE_DIR}/src/stmm-jointris-config.cc.in"
               "${PROJECT_BINARY_DIR}/stmm-jointris-config.cc" @ONLY)

include(FindPkgConfig)
if (PKG_CONFIG_FOUND)
    # Set install dir for pkg-config file
    set(STMMI_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
    # Create pkg-config file for the library
    configure_file("${PROJECT_SOURCE_DIR}/stmm-jointris.pc.in"
                   "${PROJECT_BINARY_DIR}/stmm-jointris.pc" @ONLY)
endif()

if ($ENV{STMM_CMAKE_COMMENTS})
message(STATUS "")
message(STATUS "stmm-jointris was configured with the following options:")
message(STATUS " STMMI_HEADERS:                    ${STMMI_HEADERS}")
message(STATUS " STMMI_SOURCES:                    ${STMMI_SOURCES}")
message(STATUS " STMMJOINTRIS_EXTRA_INCLUDE_DIRS:  ${STMMJOINTRIS_EXTRA_INCLUDE_DIRS}")
message(STATUS " STMMJOINTRIS_EXTRA_INCLUDE_SDIRS: ${STMMJOINTRIS_EXTRA_INCLUDE_SDIRS}")
message(STATUS " STMMJOINTRIS_EXTRA_LIBRARIES:     ${STMMJOINTRIS_EXTRA_LIBRARIES}")
message(STATUS " CMAKE_BUILD_TYPE:                 ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER_ID:            ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " CMAKE_CXX_COMPILER_VERSION:       ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " CMAKE_CXX_FLAGS:                  ${CMAKE_CXX_FLAGS}")
message(STATUS " install prefix:                   ${CMAKE_INSTALL_PREFIX}")
message(STATUS " BUILD_DOCS:                       ${BUILD_DOCS}")
message(STATUS " BUILD_TESTING:                    ${BUILD_TESTING}")
endif()

# Documentation
add_subdirectory(doc)
if (BUILD_DOCS)
    # build automatically, otherwise call `make doc`
    add_dependencies(stmm-jointris doc)
endif()

# Testing
enable_testing()
add_subdirectory(test)

install(TARGETS stmm-jointris LIBRARY DESTINATION "lib"  ARCHIVE DESTINATION "lib")

install(FILES ${STMMI_HEADERS} DESTINATION "include/stmm-jointris")

if (PKG_CONFIG_FOUND)
    install(FILES  "${PROJECT_BINARY_DIR}/stmm-jointris.pc"   DESTINATION "lib/pkgconfig")
endif()
