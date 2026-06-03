cmake_minimum_required(VERSION 3.29)
project(compiler)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQL REQUIRED mysqlclient)

include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.0
)
FetchContent_MakeAvailable(Catch2)

add_subdirectory(src/VM)
add_subdirectory(src/Lexer)
add_subdirectory(src/Parser)
add_subdirectory(src/CodeGenerator)

add_executable(
        compiler
        src/Visualizer/SchemaVisualizer.cpp
        src/Visualizer/ConsoleSchemaVisualizer.cpp
        src/main.cpp)

target_include_directories(compiler PRIVATE ${MYSQL_INCLUDE_DIRS})
target_link_libraries(
        compiler PRIVATE
        VM
        Lexer
        CodeGen
        ${MYSQL_LIBRARIES}
)
target_compile_options(compiler PRIVATE ${MYSQL_CFLAGS_OTHER})