cmake_minimum_required(VERSION 3.31)

project(Clayborne VERSION 0.1 LANGUAGES C CXX)

add_executable(clayborne)

target_sources(clayborne PRIVATE
    src/main.cpp
)

set_property(TARGET clayborne PROPERTY CXX_STANDARD 23)
set_property(TARGET clayborne PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET clayborne PROPERTY CXX_EXTENSIONS OFF)

if (MSVC)
    target_compile_options(clayborne PRIVATE
        /W4
        /WX
        $<$<CONFIG:Release>:/O2 /DNDEBUG>
    )
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
        -Wold-style-cast
        -Woverloaded-virtual
        -Wnon-virtual-dtor
        -Wzero-as-null-pointer-constant
        $<$<CONFIG:Debug>:-O0 -g>
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
    )
endif()