cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(SputnikResourcePacker VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
)

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.c"
        )

add_executable(${PROJECT_NAME} ${all_SRCS})

# Libraries
# target_link_libraries(${PROJECT_NAME}
#        libSDL2.a
#        winmm.lib
#        setupapi.lib
#        imm32.lib
#        version.lib
#        )