file(
        COPY ${CMAKE_CURRENT_SOURCE_DIR}/input_test/
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/input_test/
)


add_library(
        Lexer STATIC
        Reader.cpp
        Lexer.cpp)

add_executable(Lexer_exe Reader.cpp Lexer.cpp main.cpp)

target_include_directories(Lexer PUBLIC .)
