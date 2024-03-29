cmake_minimum_required(VERSION 3.15)

# set the project name
project(ChessEngine)
option(CHESS_ENGINE_USE_EXTERNAL_JSON "Use external Nlohmann JSON library" OFF)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
        ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

# workaround for CMP0042 warning on Mac
if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  if(NOT DEFINED CMAKE_MACOSX_RPATH)
    set(CMAKE_MACOSX_RPATH ON)
  endif()
endif()

if (POLICY CMP0042) # MACOSX_RPTH
    cmake_policy(SET CMP0042 NEW)
endif (POLICY CMP0042)

add_subdirectory(thirdparty)
add_subdirectory(src-lib)
add_subdirectory(src-exe)
add_subdirectory(tests)
add_subdirectory(benchmark)

install (TARGETS ChessEngine)
