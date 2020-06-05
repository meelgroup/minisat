cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

include (GenerateExportHeader)
include (GNUInstallDirs)
enable_language( CXX )

message(STATUS "LIB directory is '${CMAKE_INSTALL_LIBDIR}'")
message(STATUS "BIN directory is '${CMAKE_INSTALL_BINDIR}'")

if(POLICY CMP0054)
    #Something is needed to be done here
    cmake_policy(SET CMP0054 NEW)
endif()

if(POLICY CMP0048)
    #policy for VERSION in cmake 3.0
    cmake_policy(SET CMP0048 NEW)
endif()

project(minisat)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

#------------------------------------------------------------------------------
# Configurable options:
#------------------------------------------------------------------------------

option(STATIC_BINARIES "Link binaries statically." ON)
option(USE_SORELEASE   "Use SORELEASE in shared library filename." ON)

option(BIN_DRUP "Use Binay DRUP output" ON)
if (BIN_DRUP)
    add_definitions( -DBIN_DRUP )
endif()


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

#------------------------------------------------------------------------------
# Library version:
#------------------------------------------------------------------------------

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

#------------------------------------------------------------------------------
# Dependencies:
#------------------------------------------------------------------------------
find_package(ZLIB)
include_directories(${ZLIB_INCLUDE_DIR})
include_directories(${minisat_SOURCE_DIR})

#------------------------------------------------------------------------------
# Compile flags:
#------------------------------------------------------------------------------
add_definitions(-D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS)

# -----------------------------------------------------------------------------
# Look for python
# -----------------------------------------------------------------------------
if (STATICCOMPILE)
    set(ENABLE_PYTHON_INTERFACE OFF)
endif()

option(FORCE_PYTHON2 OFF)
option(FORCE_PYTHON3 OFF)

if (FORCE_PYTHON3 AND FORCE_PYTHON2)
    message(FATAL_ERROR "Cannot force both Python2 and Python3. Wrong options given.")
endif()

message(STATUS "In case your Python interpreter is not found, or a wrong one is found, please set it with '-DPYTHON_EXECUTABLE:FILEPATH=your path here'")
if (NOT FORCE_PYTHON2)
    find_package(PythonInterp 3)
    find_package(PythonLibs 3)
    if (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND AND PYTHON_INCLUDE_DIRS)
        message(STATUS "Python 3 -- PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}")
        message(STATUS "Python 3 -- PYTHON_LIBRARIES=${PYTHON_LIBRARIES}")
        message(STATUS "Python 3 -- PYTHON_INCLUDE_DIRS=${PYTHON_INCLUDE_DIRS}")
        message(STATUS "Python 3 -- PYTHONLIBS_VERSION_STRING=${PYTHONLIBS_VERSION_STRING}")
    endif()
endif()

if (NOT (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND AND PYTHON_INCLUDE_DIRS) AND FORCE_PYTHON3)
    message(FATAL_ERROR "Forced Python3 but did not find Python3")
endif()

if (NOT (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND AND PYTHON_INCLUDE_DIRS) AND NOT FORCE_PYTHON3)
    unset(PYTHONLIBS_FOUND CACHE)
    unset(PYTHON_LIBRARIES CACHE)
    unset(PYTHON_INCLUDE_PATH CACHE)
    unset(PYTHON_INCLUDE_DIRS CACHE)
    unset(PYTHON_DEBUG_LIBRARIES CACHE)
    unset(PYTHONLIBS_VERSION_STRING CACHE)

    unset(PYTHONINTERP_FOUND CACHE)
    unset(PYTHON_EXECUTABLE CACHE)
    unset(PYTHON_VERSION_STRING CACHE)

    message(STATUS "Python 3 not fully found or Python 2 has been forced -- trying Python 2.7")
    find_package(PythonInterp 2.7)
    find_package(PythonLibs 2.7)

    message(STATUS "Python 2.7 -- PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}")
    message(STATUS "Python 2.7 -- PYTHON_LIBRARIES=${PYTHON_LIBRARIES}")
    message(STATUS "Python 2.7 -- PYTHON_INCLUDE_DIRS=${PYTHON_INCLUDE_DIRS}")
    message(STATUS "Python 2.7 -- PYTHONLIBS_VERSION_STRING=${PYTHONLIBS_VERSION_STRING}")
endif()

if (NOT (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND AND PYTHON_INCLUDE_DIRS) AND FORCE_PYTHON2)
    message(FATAL_ERROR "Forced Python2 but did not find Python2")
endif()


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

# -----------------------------------------------------------------------------
# Different modes for data generation and prediction
# -----------------------------------------------------------------------------

set(lib_link_libs "")

option(PREDICT_MODE "Use final predictor" OFF)

if (PREDICT_MODE)
    find_package(dmlc REQUIRED)
    find_package(rabit REQUIRED)
    find_package(xgboost REQUIRED)
    if (xgboost_FOUND)
        message(STATUS "XGBoost -- found version ${xgboost_VERSION_MAJOR}.${xgboost_VERSION_MINOR}")
    endif()
    add_definitions( -DPREDICT_MODE )
    SET(lib_link_libs ${lib_link_libs} xgboost dmlc rabit rt)
endif()

option(STATS_MODE "Don't use SQL at all" OFF)
if (STATS_MODE)
#     if (PREDICT_MODE)
#         message(FATAL_ERROR "Cannot have stats and final predictor on both")
#     endif()
    find_package (Sqlite3)
    IF (SQLITE3_FOUND)
        MESSAGE(STATUS "OK, Found Sqlite3!")
        include_directories(${SQLITE3_INCLUDE_DIR})
        add_definitions( -DSTATS_MODE )
        SET(lib_link_libs ${lib_link_libs} ${SQLITE3_LIBRARIES})
    else ()
        message(FATAL_ERROR "Did not find Sqlite3! SQLite needed for STATS_MODE")
    endif()
ELSE ()
    MESSAGE(STATUS "Not using SQLite.")
ENDIF ()


if (SQLITE3_FOUND)
    if (NOT PYTHON_EXECUTABLE)
        MESSAGE(FATAL_ERROR "Unfortunately, the python interpreter is needed for statistics because of SQL text generation")
    endif()

    add_custom_command(
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/sql_tablestructure.cpp
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${PYTHON_EXECUTABLE} minisat/utils/xxd-alike.py tablestructure.sql ${CMAKE_CURRENT_BINARY_DIR}/sql_tablestructure.cpp
        DEPENDS ${CMAKE_SOURCE_DIR}/tablestructure.sql
    )
    add_custom_target(tablestruct ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/sql_tablestructure.cpp)
endif()

#------------------------------------------------------------------------------
# Compiling
#------------------------------------------------------------------------------
set(MINISAT_LIB_SOURCES
    minisat/utils/Options.cc
    minisat/utils/System.cc
    minisat/core/Solver.cc
    minisat/core/cl_predictors.cc
    minisat/utils/sqlitestats.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sql_tablestructure.cpp
    minisat/simp/SimpSolver.cc)



add_library(minisat-lib-static STATIC ${MINISAT_LIB_SOURCES})
add_library(minisat-lib-shared SHARED ${MINISAT_LIB_SOURCES})

target_link_libraries(minisat-lib-shared ${ZLIB_LIBRARY} ${lib_link_libs})
target_link_libraries(minisat-lib-static ${ZLIB_LIBRARY} ${lib_link_libs})

add_executable(minisat_core minisat/core/Main.cc)
add_executable(minisat_simp minisat/simp/Main.cc)


add_dependencies(minisat_simp tablestruct)

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

#------------------------------------------------------------------------------
# Installation targets:
#------------------------------------------------------------------------------
install(TARGETS minisat-lib-static minisat-lib-shared minisat_core minisat_simp
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(DIRECTORY minisat/mtl minisat/utils minisat/core minisat/simp
        DESTINATION include/minisat
        FILES_MATCHING PATTERN "*.h")
