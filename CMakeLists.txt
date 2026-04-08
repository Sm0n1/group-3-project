cmake_minimum_required(VERSION 3.22)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SDL_X11_XSCRNSAVER OFF CACHE BOOL "" FORCE)
set(SDL_X11_TEST OFF CACHE BOOL "" FORCE)
set(SDL_X11_XTEST OFF CACHE BOOL "" FORCE)

project(Clayborne VERSION 0.1 LANGUAGES C CXX)

add_executable(clayborne)

option(CLAYBORNE_VENDORED "Use vendored libraries" OFF)

if(CLAYBORNE_VENDORED)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
    add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)
else()
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
endif()

target_compile_features(clayborne PUBLIC cxx_std_23)

target_include_directories(clayborne SYSTEM PRIVATE
    vendored/entt/single_include
    vendored/SDL/include
)

target_link_libraries(clayborne PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)

target_sources(clayborne PRIVATE
    src/main.cpp
    src/camera.cpp
    src/player.cpp
    src/physics.cpp
    src/engine/input/manager.cpp
    src/engine/input/map.cpp
)

if(MSVC)
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

if(EMSCRIPTEN)
    set_target_properties(clayborne PROPERTIES
        SUFFIX ".html"
        LINK_FLAGS "-s USE_SDL=3 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=1 -s MODULARIZE=1"
    )

    target_link_options(clayborne PRIVATE "-s FORCE_FILESYSTEM=1")
endif()