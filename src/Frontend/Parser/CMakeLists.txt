add_library(
        Parser STATIC
        Grammar.cpp
        SLRTableBuilder.cpp
)

add_executable(
        Parser_exe
        Grammar.cpp
        SLRTableBuilder.cpp
        main.cpp
)

target_include_directories(Parser PUBLIC .)