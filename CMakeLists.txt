cmake_minimum_required(VERSION 3.21)
project(termcalc LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(c termcalc.c)
target_link_libraries(c PRIVATE m readline)

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
