# MaNGOS is a full featured server for World of Warcraft, supporting
# the following clients: 1.12.x, 2.4.3, 3.3.5a, 4.3.4a and 5.4.8
#
# Copyright (C) 2005-2021 MaNGOS <https://getmangos.eu>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# CMAKE Version.
cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Project Name
project(MaNGOS_Extractors VERSION 0.22.0)

# Flags
cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0063 NEW)

set(CMAKE_CXX_STANDARD          11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

set_property(GLOBAL
    PROPERTY USE_FOLDERS ON
)

#==================================================================================
# Define available cmake options below
option(DEBUG                "Enable debug build (only on non IDEs)"         OFF)
#==================================================================================
message("")
message(
  "This script builds the MaNGOS extractor.
  Options that can be used in order to configure the process:
   General:
    CMAKE_INSTALL_PREFIX    Path where the server should be installed to
    DEBUG                   Debug build, only for systems without IDE (Linux, *BSD)
  To set an option simply type -D<OPTION>=<VALUE> after 'cmake <srcs>'.
  Also, you can specify the generator with -G. See 'cmake --help' for more details
  For example: cmake .. -DCMAKE_INSTALL_PREFIX=/opt/mangos"
)
message("")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Default install directory" FORCE)
endif()

if(DEBUG)
  set(CMAKE_BUILD_TYPE Debug)
else()
  set(CMAKE_BUILD_TYPE Release)
endif()

if(UNIX)
    set(BIN_DIR ${CMAKE_INSTALL_PREFIX}/bin)
    if (NOT CONF_INSTALL_DIR)
        set(CONF_INSTALL_DIR ../etc)
    endif()
else()
    set(BIN_DIR ${CMAKE_INSTALL_PREFIX})
    if (NOT CONF_INSTALL_DIR)
        set(CONF_INSTALL_DIR .)
    endif()
endif()

# Used for install targets
set(EXTRACTOR_BINARIES_DIR "${CMAKE_SOURCE_DIR}/Extractor_Binaries")

# External Libs
## POCO
add_subdirectory(./libs/poco)
## StormLib
add_subdirectory(./libs/StormLib)

#=======================================================#
#extractor
#=======================================================#

#Main group
file(GLOB SRC_GRP_MAIN src/*.cpp src/*.h)
source_group("main" FILES ${SRC_GRP_MAIN})

#Common Client group
file(GLOB SRC_GRP_CLIENT src/client/*.cpp src/client/*.h)
source_group("client" FILES ${SRC_GRP_CLIENT})

#Geometry group
file(GLOB SRC_GRP_GEOMETRY src/geometry/*.h)
source_group("geometry" FILES ${SRC_GRP_GEOMETRY})

#MPQ Files Client group
file(GLOB SRC_GRP_CLIENT_MPQ src/client/mpq/*.cpp src/client/mpq/*.h)
source_group("client\\mpq" FILES ${SRC_GRP_CLIENT_MPQ})

#Client Classic group
file(GLOB SRC_GRP_CLIENT_CLASSIC src/client/classic/*.cpp src/client/classic/*.h)
source_group("client\\classic" FILES ${SRC_GRP_CLIENT_CLASSIC})

#Client Bruning Crusade group
file(GLOB SRC_GRP_CLIENT_TBC src/client/burningcrusade/*.cpp src/client/burningcrusade/*.h)
source_group("client\\burningcrusade" FILES ${SRC_GRP_CLIENT_TBC})

#Client Wrath of the Lich King group
file(GLOB SRC_GRP_CLIENT_WOTLK src/client/wrathlichking/*.cpp src/client/wrathlichking/*.h)
source_group("client\\wrathlichking" FILES ${SRC_GRP_CLIENT_WOTLK})

#Client Cataclysm group
file(GLOB SRC_GRP_CLIENT_CATA src/client/cataclysm/*.cpp src/client/cataclysm/*.h)
source_group("client\\cataclysm" FILES ${SRC_GRP_CLIENT_CATA})

#Map File group
file(GLOB SRC_GRP_MAP_FILES src/maps/*.cpp src/maps/*.h)
source_group("maps" FILES ${SRC_GRP_MAP_FILES})

#Vmap File group
file(GLOB SRC_GRP_VMAP_FILES src/vmaps/*.cpp src/vmaps/*.h)
source_group("vmaps" FILES ${SRC_GRP_VMAP_FILES})

add_executable(extractor
    ${SRC_GRP_MAIN}
    ${SRC_GRP_GEOMETRY}
    ${SRC_GRP_MAP_FILES}
    ${SRC_GRP_VMAP_FILES}
    ${SRC_GRP_CLIENT}
    ${SRC_GRP_CLIENT_CLASSIC}
    ${SRC_GRP_CLIENT_TBC}
    ${SRC_GRP_CLIENT_WOTLK}
    ${SRC_GRP_CLIENT_CATA}
    ${SRC_GRP_CLIENT_MPQ}
    $<$<BOOL:${WIN32}>:src/extractor.rc>
)

target_link_libraries(extractor
    PUBLIC
    storm
    Poco::Foundation
    Poco::Util
)

install(
    TARGETS extractor
    DESTINATION "${BIN_DIR}/"
)

if(WIN32 AND MSVC)
    install(
        FILES $<TARGET_PDB_FILE:extractor>
        DESTINATION ${BIN_DIR}/
        OPTIONAL)
endif()

install(
  DIRECTORY
    "${EXTRACTOR_BINARIES_DIR}/"
  DESTINATION
    "${BIN_DIR}"
    )