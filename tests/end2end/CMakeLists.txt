unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY)

file(
        COPY ${CMAKE_CURRENT_SOURCE_DIR}/data/
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/data/
)

add_executable(
        TestCompiler
        ../../src/Visualizer/SchemaVisualizer.cpp
        ../../src/Visualizer/ConsoleSchemaVisualizer.cpp
        ../../src/Engine/Engine.cpp
        TestCompiler.cpp)

target_link_libraries(
        TestCompiler PRIVATE
        VM
        Lexer
        CodeGen
        CURL::libcurl
        ${MYSQL_LIBRARIES}
)
target_compile_definitions(TestCompiler PRIVATE GRAMMAR_PATH="${GRAMMAR_PATH}")
target_link_libraries(TestCompiler PRIVATE Catch2::Catch2WithMain)
