add_library(
        CodeGen STATIC
        ../Frontend/SematicAnalyzer/SemanticAnalyzer.cpp
        CodeGenerator.cpp
)

target_include_directories(CodeGen PUBLIC .)

target_link_libraries(CodeGen PRIVATE Parser VM)