cmake_minimum_required(VERSION 4.0)
project(compiler)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.0
)
FetchContent_MakeAvailable(Catch2)

add_subdirectory(src/VM)

add_executable(compiler src/main.cpp)

target_link_libraries(compiler PRIVATE VM)
