cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

include (GenerateExportHeader)
include (GNUInstallDirs)
enable_language( CXX )

message(STATUS "LIB directory is '${CMAKE_INSTALL_LIBDIR}'")
message(STATUS "BIN directory is '${CMAKE_INSTALL_BINDIR}'")


project(minisat)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

#--------------------------------------------------------------------------------------------------
# Configurable options:

option(STATIC_BINARIES "Link binaries statically." ON)
option(USE_SORELEASE   "Use SORELEASE in shared library filename." ON)

include(CheckCXXCompilerFlag)
macro(add_cxx_flag_if_supported flagname)
  check_cxx_compiler_flag("${flagname}" HAVE_FLAG_${flagname})

  if(HAVE_FLAG_${flagname})
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flagname}" )
  endif()
endmacro()

add_cxx_flag_if_supported("-Wall")
add_cxx_flag_if_supported("-Wextra")
add_cxx_flag_if_supported("-Wunused")
add_cxx_flag_if_supported("-Wsign-compare")
if (NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_cxx_flag_if_supported("-fno-omit-frame-pointer")
endif()
add_cxx_flag_if_supported("-Wtype-limits")
add_cxx_flag_if_supported("-Wuninitialized")
add_cxx_flag_if_supported("-Wno-deprecated")
add_cxx_flag_if_supported("-Wstrict-aliasing")
add_cxx_flag_if_supported("-Wpointer-arith")
add_cxx_flag_if_supported("-Wheader-guard")
add_cxx_flag_if_supported("-Wpointer-arith")
add_cxx_flag_if_supported("-Wformat-nonliteral")
add_cxx_flag_if_supported("-Winit-self")
add_cxx_flag_if_supported("-Wparentheses")
add_cxx_flag_if_supported("-Wunreachable-code")
add_cxx_flag_if_supported("-ggdb3")

#--------------------------------------------------------------------------------------------------
# Library version:

set(MINISAT_SOMAJOR   2)
set(MINISAT_SOMINOR   1)
set(MINISAT_SORELEASE 0)

# Compute VERSION and SOVERSION:
if (USE_SORELEASE)
  set(MINISAT_VERSION ${MINISAT_SOMAJOR}.${MINISAT_SOMINOR}.${MINISAT_SORELEASE})
else()
  set(MINISAT_VERSION ${MINISAT_SOMAJOR}.${MINISAT_SOMINOR})
endif()
set(MINISAT_SOVERSION ${MINISAT_SOMAJOR})

#--------------------------------------------------------------------------------------------------
# Dependencies:

find_package(ZLIB)
include_directories(${ZLIB_INCLUDE_DIR})
include_directories(${minisat_SOURCE_DIR})

#--------------------------------------------------------------------------------------------------
# Compile flags:

add_definitions(-D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS)

# -----------------------------------------------------------------------------
# Add GIT version
# -----------------------------------------------------------------------------
find_program (GIT_EXECUTABLE git)
if (GIT_EXECUTABLE)
  include(GetGitRevisionDescription)
  get_git_head_revision(GIT_REFSPEC GIT_SHA1)
  MESSAGE(STATUS "GIT hash found: ${GIT_SHA1}")
else()
  set(GIT_SHA "GIT-hash-notfound")
endif()

option(PREDICT_MODE "Use final predictor" OFF)

if (PREDICT_MODE)
    find_package(dmlc REQUIRED)
    find_package(rabit REQUIRED)
    find_package(xgboost REQUIRED)
    if (xgboost_FOUND)
        message(STATUS "XGBoost -- found version ${xgboost_VERSION_MAJOR}.${xgboost_VERSION_MINOR}")
    endif()
    add_definitions( -DPREDICT_MODE )
endif()

option(STATS_MODE "Don't use SQL at all" OFF)
if (STATS_MODE)
    if (PREDICT_MODE)
        message(FATAL_ERROR "Cannot have stats and final predictor on both")
    endif()
    find_package (Sqlite3)
    IF (SQLITE3_FOUND)
        MESSAGE(STATUS "OK, Found Sqlite3!")
        include_directories(${SQLITE3_INCLUDE_DIR})
        add_definitions( -DSTATS_MODE )
    else ()
        message(FATAL_ERROR "Did not find Sqlite3! SQLite needed for STATS_MODE")
    endif()
ELSE ()
    MESSAGE(STATUS "Not using SQLite.")
ENDIF ()


option(BIN_DRUP "Use Binay DRUP output" ON)
if (BIN_DRUP)
    add_definitions( -DBIN_DRUP )
endif()

#--------------------------------------------------------------------------------------------------
# Build Targets:

set(MINISAT_LIB_SOURCES
    minisat/utils/Options.cc
    minisat/utils/System.cc
    minisat/core/Solver.cc
    minisat/core/cl_predictors.cc
    minisat/utils/sqlitestats.cpp
#     ${CMAKE_CURRENT_BINARY_DIR}/sql_tablestructure.cpp
    minisat/simp/SimpSolver.cc)

set(cryptoms_lib_link_libs "")

if (FINAL_PREDICTOR)
    SET(cryptoms_lib_link_libs ${cryptoms_lib_link_libs} xgboost dmlc rabit rt)
endif()

if (SQLITE3_FOUND)
    MESSAGE(STATUS "OK, Found Sqlite3! " ${SQLITE3_LIBRARIES})
    SET(cryptoms_lib_link_libs ${cryptoms_lib_link_libs} ${SQLITE3_LIBRARIES})
endif ()

add_library(minisat-lib-static STATIC ${MINISAT_LIB_SOURCES})
add_library(minisat-lib-shared SHARED ${MINISAT_LIB_SOURCES})

target_link_libraries(minisat-lib-shared ${ZLIB_LIBRARY} xgboost dmlc rabit rt ${SQLITE3_LIBRARIES})
target_link_libraries(minisat-lib-static ${ZLIB_LIBRARY} xgboost dmlc rabit rt ${SQLITE3_LIBRARIES})

add_executable(minisat_core minisat/core/Main.cc)
add_executable(minisat_simp minisat/simp/Main.cc)

if(STATIC_BINARIES)
  target_link_libraries(minisat_core minisat-lib-static)
  target_link_libraries(minisat_simp minisat-lib-static)
else()
  target_link_libraries(minisat_core minisat-lib-shared)
  target_link_libraries(minisat_simp minisat-lib-shared)
endif()

set_target_properties(minisat-lib-static PROPERTIES OUTPUT_NAME "minisat")
set_target_properties(minisat-lib-shared
  PROPERTIES
    OUTPUT_NAME "minisat"
    VERSION ${MINISAT_VERSION}
    SOVERSION ${MINISAT_SOVERSION})

set_target_properties(minisat_simp       PROPERTIES OUTPUT_NAME "minisat")

#--------------------------------------------------------------------------------------------------
# Installation targets:

install(TARGETS minisat-lib-static minisat-lib-shared minisat_core minisat_simp
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(DIRECTORY minisat/mtl minisat/utils minisat/core minisat/simp
        DESTINATION include/minisat
        FILES_MATCHING PATTERN "*.h")
