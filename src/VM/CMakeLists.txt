add_library(VM STATIC
        StringPool/StringPool.cpp
        Parser/BytecodeParser.cpp
        NativeLib/http/http_client.cpp
        NativeLib/http/http_server.cpp
        NativeLib/sql/sql.cpp
        NativeLib/json/json.cpp
        NativeLib/base/base.cpp
        VirtualMachine.cpp
)
target_include_directories(VM PUBLIC .)


add_executable(VM_exe
        StringPool/StringPool.cpp
        Parser/BytecodeParser.cpp
        NativeLib/http/http_client.cpp
        NativeLib/http/http_server.cpp
        NativeLib/sql/sql.cpp
        NativeLib/json/json.cpp
        NativeLib/base/base.cpp
        VirtualMachine.cpp
        main.cpp)

add_executable(TestVM_Unit
        StringPool/StringPool.cpp
        Parser/BytecodeParser.cpp
        NativeLib/http/http_client.cpp
        NativeLib/http/http_server.cpp
        NativeLib/sql/sql.cpp
        NativeLib/json/json.cpp
        NativeLib/base/base.cpp
        VirtualMachine.cpp
        tests/Unit.cpp
)

target_link_libraries(TestVM_Unit PRIVATE Catch2::Catch2WithMain)

target_compile_definitions(VM_exe PRIVATE VM_STANDALONE_MODE)
