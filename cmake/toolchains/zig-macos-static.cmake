# Toolchain for building macOS static binaries using Zig's clang frontend

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

if(NOT ZIG_EXECUTABLE)
  if(DEFINED ENV{ZIG} AND NOT "$ENV{ZIG}" STREQUAL "")
    set(ZIG_EXECUTABLE "$ENV{ZIG}")
  else()
    find_program(ZIG_EXECUTABLE zig REQUIRED)
  endif()
endif()

set(ZIG_EXECUTABLE "${ZIG_EXECUTABLE}" CACHE FILEPATH "Path to the zig compiler" FORCE)

set(ENV{ZIG} "${ZIG_EXECUTABLE}")
set(ENV{ZIG_EXECUTABLE} "${ZIG_EXECUTABLE}")
get_filename_component(_zig_dir "${ZIG_EXECUTABLE}" DIRECTORY)
if(NOT _zig_dir STREQUAL "")
  set(ENV{PATH} "${_zig_dir}:$ENV{PATH}")
endif()

set(_zig_wrapper_dir "${CMAKE_BINARY_DIR}/zig-toolchain")
file(MAKE_DIRECTORY "${_zig_wrapper_dir}")

set(_zig_cc "${_zig_wrapper_dir}/zig-cc")
file(WRITE "${_zig_cc}" "#!/usr/bin/env bash\n"
                 "set -euo pipefail\n"
                 "args=()\n"
                 "for arg in \"\$@\"; do\n"
                 "  case \"\$arg\" in\n"
                 "    -v|-Wl,-v)\n"
                 "      ;;\n"
                 "    *)\n"
                 "      args+=(\"\$arg\")\n"
                 "      ;;\n"
                 "  esac\n"
                 "done\n"
                 "exec \"${ZIG_EXECUTABLE}\" cc \"\${args[@]}\"\n")
file(CHMOD "${_zig_cc}" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

set(_zig_cxx "${_zig_wrapper_dir}/zig-cxx")
file(WRITE "${_zig_cxx}" "#!/usr/bin/env bash\n"
                  "set -euo pipefail\n"
                  "args=()\n"
                  "for arg in \"\$@\"; do\n"
                  "  case \"\$arg\" in\n"
                  "    -v|-Wl,-v)\n"
                  "      ;;\n"
                  "    *)\n"
                  "      args+=(\"\$arg\")\n"
                  "      ;;\n"
                  "  esac\n"
                  "done\n"
                  "exec \"${ZIG_EXECUTABLE}\" c++ \"\${args[@]}\"\n")
file(CHMOD "${_zig_cxx}" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

set(CMAKE_C_COMPILER "${_zig_cc}")
set(CMAKE_CXX_COMPILER "${_zig_cxx}")

set(CMAKE_C_COMPILER_TARGET "x86_64-macos")
set(CMAKE_CXX_COMPILER_TARGET "x86_64-macos")
set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version" FORCE)

set(CMAKE_C_FLAGS_INIT "-target x86_64-macos")
set(CMAKE_CXX_FLAGS_INIT "-target x86_64-macos")
if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
  set(_zig_link_flags "-target x86_64-macos")
else()
  set(_zig_link_flags "-target x86_64-macos -static")
endif()

set(CMAKE_EXE_LINKER_FLAGS_INIT "${_zig_link_flags}")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
