add_library(
        Lexer STATIC
        Reader.cpp
        Lexer.cpp)

add_executable(Lexer_exe Reader.cpp Lexer.cpp main.cpp)

target_include_directories(Lexer PUBLIC .)
