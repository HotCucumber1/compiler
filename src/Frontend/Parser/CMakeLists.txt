add_library(
        Parser STATIC
        Grammar/Grammar.cpp
        Grammar/GrammarReader.cpp
        Common/Node.cpp
        SLRTableBuilder.cpp
        AstFactory.cpp
        Common/AstPrinter.cpp
        Parser.cpp
)

add_executable(
        Parser_exe
        Grammar/Grammar.cpp
        Grammar/GrammarReader.cpp
        Common/Node.cpp
        SLRTableBuilder.cpp
        AstFactory.cpp
        Common/AstPrinter.cpp
        Parser.cpp
        main.cpp
)

target_include_directories(Parser PUBLIC .)

target_link_libraries(Parser PUBLIC Lexer)
target_link_libraries(Parser_exe PUBLIC Lexer)
