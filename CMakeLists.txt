cmake_minimum_required(VERSION 3.15)

# set the project name
project(ChessEngine)

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

add_subdirectory(src-lib)
add_subdirectory(src-exe)
add_subdirectory(tests)

install (TARGETS ChessEngine)
