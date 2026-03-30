cmake_minimum_required(VERSION 3.31)

project(Clayborne VERSION 0.1 LANGUAGES C CXX)

add_executable(clayborne)

target_sources(clayborne PRIVATE
    src/main.cpp
)

target_compile_options(clayborne PRIVATE -std=c++23)

if (MSVC)
    target_compile_options(clayborne PRIVATE /W4 /WX)
else()
    target_compile_options(clayborne PRIVATE
        -Wall
        -Wextra
        -Werror
        -Wshadow
        -Wconversion
        -Wdouble-promotion
        -Wformat=2
        -Wundef
        -Wnull-dereference
        -Wpedantic
        -Wimplicit-fallthrough
        -Wmissing-declarations
        -Wcast-qual
        -Wwrite-strings
        -Wold-style-cast
        -Woverloaded-virtual
        -Wnon-virtual-dtor
        -Wzero-as-null-pointer-constant
    )
endif()

# Add different builds