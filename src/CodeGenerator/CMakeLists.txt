add_library(
        CodeGen STATIC
        CodeGenerator.cpp
        ASM_CodeGenerator.cpp
)

target_include_directories(CodeGen PUBLIC .)

target_link_libraries(CodeGen PRIVATE Parser VM)