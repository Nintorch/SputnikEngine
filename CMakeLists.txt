cmake_minimum_required(VERSION 3.10)

# TODO: reconstruct the file
# set the project name and version
project(SputnikEngine VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/libs/SDL2
    ${PROJECT_SOURCE_DIR}/include/libs/SDL_gpu
    ${PROJECT_SOURCE_DIR}/include/libs/SoLoud
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/platform
    ${PROJECT_SOURCE_DIR}/src/gamefiles
    ${PROJECT_SOURCE_DIR}/src/gamefiles/scenes
)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

file(GLOB all_SRCS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.c"

    "${PROJECT_SOURCE_DIR}/src/SoLoud/*/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/SoLoud/*/*.c"

    "${PROJECT_SOURCE_DIR}/src/SoLoud/*/*/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/SoLoud/*/*/*.c"

    "${PROJECT_SOURCE_DIR}/src/gamefiles/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/gamefiles/cpp_resources/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/gamefiles/scenes/*.cpp"

    "${PROJECT_SOURCE_DIR}/src/renderers/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/platform/*.cpp"
    )

add_executable(${PROJECT_NAME} ${all_SRCS})

set(LIBS
    # put other libraries here
    )

set(LIBS ${LIBS}
    libSDL2.a
    libSDL2_image.a
    libSDL2_ttf.a
)

# Set to false to remove SDL gpu support where possible
set(SDL_GPU true)

add_compile_definitions(${PROJECT_NAME} _DEBUG)

if (WIN32)
    # SoLoud backends
    add_compile_definitions(${PROJECT_NAME}
        WITH_SDL2_STATIC
        WITH_WINMM
        WITH_MINIAUDIO)

    if (SDL_GPU)
        set(LIBS ${LIBS} libSDL2_gpu.a)
    endif()

    set(LIBS ${LIBS}
        winmm.lib
        setupapi.lib
        imm32.lib
        version.lib
        libopengl32.a
        dwmapi.lib
        Rpcrt4.lib
    )
endif()

# Libraries
target_link_libraries(${PROJECT_NAME} ${LIBS})