#include "../Parser/BytecodeParser.h"
#include "../Type/Value.h"
#include "../VirtualMachine.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>

void RunBytecode(VirtualMachine& vm, const std::string& source)
{
	auto chunk = BytecodeParser::Parse(source);
	vm.Interpret(std::move(chunk));
}

struct VMAccessor
{
	static Value GetStackTop(const VirtualMachine& vm)
	{
		return vm.GetStackTop();
	}
	static Value GetLocal(const VirtualMachine& vm, const size_t index)
	{
		return vm.GetLocal(index);
	}
};

// ==========================================
// 1. BASIC ARITHMETIC (INTEGERS)
// ==========================================
TEST_CASE("VM: Integer Arithmetic", "[vm][math][int]")
{
	VirtualMachine vm;

	SECTION("Addition")
	{
		std::string code = R"(
            .locals 0
            .const
            int 12
            int 34
            .code
            push 0
            push 1
            add
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		REQUIRE(AsInt(top) == 46);
	}

	SECTION("Subtraction and Operand Order")
	{
		// 10 - 3 = 7. Ensures Pop() order is correct (right operand first)
		std::string code = R"(
            .locals 0
            .const
            int 10
            int 3
            .code
            push 0
            push 1
            sub
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		REQUIRE(AsInt(top) == 7);
	}

	SECTION("Multiplication")
	{
		std::string code = R"(
            .locals 0
            .const
            int 5
            int 6
            .code
            push 0
            push 1
            multi
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		REQUIRE(AsInt(top) == 30);
	}

	SECTION("Division and Operand Order")
	{
		// 20 / 4 = 5
		std::string code = R"(
            .locals 0
            .const
            int 20
            int 4
            .code
            push 0
            push 1
            div
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		REQUIRE(AsInt(top) == 5);
	}

	SECTION("Modulo")
	{
		// 17 % 5 = 2
		std::string code = R"(
            .locals 0
            .const
            int 17
            int 5
            .code
            push 0
            push 1
            mod
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		REQUIRE(AsInt(top) == 2);
	}

	SECTION("Unary Negation")
	{
		std::string code = R"(
            .locals 0
            .const
            int 42
            .code
            push 0
            neg
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		REQUIRE(AsInt(top) == -42);
	}
}

// ==========================================
// 2. FLOATING POINT ARITHMETIC
// ==========================================
TEST_CASE("VM: Double Arithmetic", "[vm][math][double]")
{
	VirtualMachine vm;

	SECTION("Addition with Doubles")
	{
		std::string code = R"(
            .locals 0
            .const
            double 12.5
            double 3.2
            .code
            push 0
            push 1
            add
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		REQUIRE(AsDouble(top) == Catch::Approx(15.7));
	}

	SECTION("Mixed Types Addition (Int + Double)")
	{
		// Assuming your std::visit OpAdd implementation promotes Int to Double
		std::string code = R"(
            .locals 0
            .const
            int 10
            double 5.5
            .code
            push 0
            push 1
            add
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		REQUIRE(AsDouble(top) == Catch::Approx(15.5));
	}
}

// ==========================================
// 3. LOCAL VARIABLES
// ==========================================
TEST_CASE("VM: Local Variables", "[vm][locals]")
{
	VirtualMachine vm; // Provide space for 2 locals

	SECTION("Set and Get Local Variable")
	{
		std::string code = R"(
            .locals 2
            .const
            int 99
            .code
            push 0
            set_local_var 0
            get_local_var 0
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		auto local = VMAccessor::GetLocal(vm, 0);
		// Stack should contain 99 after get_local_var
		REQUIRE(AsInt(top) == 99);
		// The local storage should also hold 99
		REQUIRE(AsInt(local) == 99);
	}

	SECTION("Move Value Between Locals")
	{
		std::string code = R"(
            .locals 2
            .const
            int 42
            .code
            push 0
            set_local_var 0 ; var[0] = 42
            get_local_var 0 ; push var[0]
            set_local_var 1 ; var[1] = pop()
            return
        )";
		RunBytecode(vm, code);
		auto local0 = VMAccessor::GetLocal(vm, 0);
		auto local1 = VMAccessor::GetLocal(vm, 1);
		REQUIRE(AsInt(local0) == 42);
		REQUIRE(AsInt(local1) == 42);
	}
}

// ==========================================
// 4. COMPLEX EXPRESSIONS (ORDER OF OPERATIONS)
// ==========================================
TEST_CASE("VM: Complex Expressions", "[vm][execution]")
{
	VirtualMachine vm;

	SECTION("Evaluate: (10 - 2) * 3 = 24")
	{
		std::string code = R"(
            .locals 0
            .const
            int 10
            int 2
            int 3
            .code
            push 0
            push 1
            sub
            push 2
            multi
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		REQUIRE(AsInt(top) == 24);
	}
}

// ==========================================
// 5. ERROR HANDLING & EDGE CASES
// ==========================================
TEST_CASE("VM: Error Handling and Edge Cases", "[vm][errors]")
{
	VirtualMachine vm; // 1 local var max

	SECTION("Stack Underflow on Math Operation")
	{
		std::string code = R"(
            .locals 0
            .const
            int 10
            .code
            push 0
            add     ; Fails: needs 2 items, only 1 on stack
            return
        )";
		REQUIRE_THROWS_AS(RunBytecode(vm, code), std::runtime_error);
	}

	SECTION("Stack Underflow on set_local_var")
	{
		std::string code = R"(
            .locals 1
            .const
            .code
            set_local_var 0 ; Fails: nothing on stack to set
            return
        )";
		REQUIRE_THROWS_AS(RunBytecode(vm, code), std::runtime_error);
	}

	SECTION("Division by Zero")
	{
		std::string code = R"(
            .locals 0
            .const
            int 15
            int 0
            .code
            push 0
            push 1
            div
            return
        )";
		REQUIRE_THROWS_AS(RunBytecode(vm, code), std::runtime_error);
	}

	SECTION("Local Variable Out of Bounds")
	{
		std::string code = R"(
            .locals 1
            .const
            int 10
            .code
            push 0
            set_local_var 5 ; Fails: max index is 0
            return
        )";
		REQUIRE_THROWS_AS(RunBytecode(vm, code), std::runtime_error);
	}

	SECTION("Type Mismatch (Invalid Addition)")
	{
		// Assumes adding a bool and an int throws a runtime_error in your std::visit
		std::string code = R"(
            .locals 0
            .const
            bool true
            int 5
            .code
            push 0
            push 1
            add
            return
        )";
		REQUIRE_THROWS_AS(RunBytecode(vm, code), std::runtime_error);
	}
}