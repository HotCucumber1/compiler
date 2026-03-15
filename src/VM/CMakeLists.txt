include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(VM
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
