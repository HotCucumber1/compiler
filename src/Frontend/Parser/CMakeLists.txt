add_library(
        Parser STATIC
        Common/Grammar.cpp
        SLRTableBuilder.cpp
        Parser.cpp
)

add_executable(
        Parser_exe
        Common/Grammar.cpp
        SLRTableBuilder.cpp
        Parser.cpp
        main.cpp
)

target_include_directories(Parser PUBLIC .)

target_link_libraries(Parser PUBLIC Lexer)
target_link_libraries(Parser_exe PUBLIC Lexer)
