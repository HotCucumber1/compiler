add_library(VM STATIC
        StringPool/StringPool.cpp
        Parser/BytecodeParser.cpp
        VirtualMachine.cpp
)
target_include_directories(VM PUBLIC .)


add_executable(VM_exe
        StringPool/StringPool.cpp
        Parser/BytecodeParser.cpp
        VirtualMachine.cpp
        main.cpp)

add_executable(TestVM_Unit
        StringPool/StringPool.cpp
        Parser/BytecodeParser.cpp
        VirtualMachine.cpp
        tests/Unit.cpp
)

target_link_libraries(TestVM_Unit PRIVATE Catch2::Catch2WithMain)
