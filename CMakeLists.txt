cmake_minimum_required(VERSION 3.21)

# Select clang > 16 for aarch64 (must be before project())
execute_process(
    COMMAND uname -m
    OUTPUT_VARIABLE HOST_ARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(HOST_ARCH MATCHES "aarch64|arm64")
    # Find clang versions > 16
    find_program(CLANG_20 clang-20)
    find_program(CLANG_19 clang-19)
    find_program(CLANG_18 clang-18)
    find_program(CLANG_17 clang-17)
    find_program(CLANG_GENERIC clang)

    # Prefer newer versions
    if(CLANG_20)
        set(CMAKE_C_COMPILER ${CLANG_20})
    elseif(CLANG_19)
        set(CMAKE_C_COMPILER ${CLANG_19})
    elseif(CLANG_18)
        set(CMAKE_C_COMPILER ${CLANG_18})
    elseif(CLANG_17)
        set(CMAKE_C_COMPILER ${CLANG_17})
    elseif(CLANG_GENERIC)
        # Check generic clang version
        execute_process(
            COMMAND ${CLANG_GENERIC} --version
            OUTPUT_VARIABLE CLANG_VERSION_OUTPUT
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        string(REGEX MATCH "[0-9]+" CLANG_MAJOR_VERSION "${CLANG_VERSION_OUTPUT}")
        if(CLANG_MAJOR_VERSION GREATER 16)
            set(CMAKE_C_COMPILER ${CLANG_GENERIC})
        endif()
    endif()

    if(CMAKE_C_COMPILER)
        message(STATUS "Selected ${CMAKE_C_COMPILER} for aarch64")
    else()
        message(WARNING "No clang > 16 found for aarch64, using default compiler")
    endif()
endif()

project(termcalc LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Find libreadline
find_library(READLINE_LIBRARY readline REQUIRED)
find_path(READLINE_INCLUDE_DIR readline/readline.h REQUIRED)

if(NOT READLINE_LIBRARY)
    message(FATAL_ERROR "libreadline not found. Please install libreadline-dev")
endif()

message(STATUS "Found readline: ${READLINE_LIBRARY}")

add_executable(c termcalc.c)
target_include_directories(c PRIVATE ${READLINE_INCLUDE_DIR})
target_link_libraries(c PRIVATE m ${READLINE_LIBRARY})

# Optimize for speed
target_compile_options(c PRIVATE
    -O3
    -march=native
    -flto
    -Wall
    -Wextra
    -Wpedantic
)

target_link_options(c PRIVATE -flto -s)

# Install to ~/.local/bin
install(TARGETS c DESTINATION $ENV{HOME}/.local/bin)
