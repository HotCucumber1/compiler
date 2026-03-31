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
		REQUIRE(AsInt(top) == 99);
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
			get_local_var 0
            return
        )";
		RunBytecode(vm, code);
		auto local0 = VMAccessor::GetStackTop(vm);
		REQUIRE(AsInt(local0) == 42);
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

// ==========================================
// 6. ARRAYS
// ==========================================
TEST_CASE("VM: Arrays", "[vm][arrays]")
{
	VirtualMachine vm;

	SECTION("Create array and get element")
	{
		std::string code = R"(
            .locals 0
            .const
            int 10
            int 20
            int 30
			int 1
            .code
            push 0
            push 1
            push 2
            create_arr 3
            push 3
            get_el
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		REQUIRE(AsInt(top) == 20);
	}

	SECTION("Create array and get last element")
	{
		std::string code = R"(
            .locals 0
            .const
            int 5
            int 15
            int 25
			int 2
            .code
            push 0
            push 1
            push 2
            create_arr 3
            push 3
            get_el
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		REQUIRE(AsInt(top) == 25);
	}

	SECTION("Set array element")
	{
		std::string code = R"(
            .locals 1
            .const
            int 1
            int 2
            int 3
            int 99
            .code
            push 0   ; 1
            push 1   ; 2
            push 2   ; 3
            create_arr 3      ; array = [1,2,3]
            set_local_var 0   ; сохраняем массив
            push 3            ; 99 (значение)
            get_local_var 0   ; массив
            push 1            ; индекс
            set_el            ; array[1] = 99
            get_local_var 0   ; массив
            push 1            ; индекс
            get_el
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		REQUIRE(AsInt(top) == 99);
	}

	SECTION("Out of bounds returns null")
	{
		std::string code = R"(
            .locals 0
            .const
            int 1
            int 2
			int 5
            .code
            push 0
            push 1
            create_arr 2
            push 2
            get_el
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		REQUIRE(IsNull(top));
	}

	SECTION("Negative index returns null")
	{
		std::string code = R"(
            .locals 0
            .const
            int 1
            int 2
			int -1
            .code
            push 0
            push 1
            create_arr 2
            push 2
            get_el
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		REQUIRE(IsNull(top));
	}

	SECTION("Set element out of bounds throws")
	{
		std::string code = R"(
            .locals 1
            .const
            int 1
            int 2
            int 99
			int 10
            .code
            push 0
            push 1
            create_arr 2
            set_local_var 0
            push 2            ; значение 99
            get_local_var 0   ; массив
            push 3           ; индекс
            set_el
            return
        )";
		REQUIRE_THROWS_AS(RunBytecode(vm, code), std::runtime_error);
	}

	SECTION("Set element with negative index throws")
	{
		std::string code = R"(
            .locals 1
            .const
            int 1
            int 2
            int 99
			int -1
            .code
            push 0
            push 1
            create_arr 2
            set_local_var 0
            push 2
            get_local_var 0
            push 3
            set_el
            return
        )";
		REQUIRE_THROWS_AS(RunBytecode(vm, code), std::runtime_error);
	}

	SECTION("Create empty array")
	{
		std::string code = R"(
            .locals 0
            .const
			int 0
            .code
            create_arr 0
            push 0
            get_el
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		REQUIRE(IsNull(top));
	}

	SECTION("Array of mixed types")
	{
		std::string code = R"(
            .locals 0
            .const
            int 42
            double 3.14
            bool true
            string "hello"
			int 1
            .code
            push 0   ; 42
            push 1   ; 3.14
            push 2   ; true
            push 3   ; "hello"
            create_arr 4
            push 4   ; индекс 1 (double 3.14)
            get_el
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		REQUIRE(AsDouble(top) == Catch::Approx(3.14));
	}

	SECTION("Array of arrays (nested)")
	{
		std::string code = R"(
            .locals 2
            .const
            int 1
            int 2
            int 3
            int 4
            .code
            push 0
            push 1
            create_arr 2
            set_local_var 0

            get_local_var 0
            push 2
            push 3
            create_arr 3
            set_local_var 1

            ; get element at index 1 from outer array (should be 3)
            get_local_var 1
            push 0
            get_el
            return
        )";
		RunBytecode(vm, code);
		auto top = VMAccessor::GetStackTop(vm);
		REQUIRE(AsInt(top) == 3);
	}

	SECTION("Get element from non-array throws")
	{
		std::string code = R"(
            .locals 0
            .const
            int 42
            .code
            push 0
            push 0
            get_el
            return
        )";
		REQUIRE_THROWS_AS(RunBytecode(vm, code), std::runtime_error);
	}

	SECTION("Set element on non-array throws")
	{
		std::string code = R"(
            .locals 0
            .const
            int 42
            .code
            push 0
            push 0
            push 0
            set_el
            return
        )";
		REQUIRE_THROWS_AS(RunBytecode(vm, code), std::runtime_error);
	}

	SECTION("Index not integer throws")
	{
		std::string code = R"(
            .locals 0
            .const
            int 1
            int 2
            double 3.14
            .code
            push 0
            push 1
            create_arr 2
            push 2   ; double 3.14 as index
            get_el
            return
        )";
		REQUIRE_THROWS_AS(RunBytecode(vm, code), std::runtime_error);
	}
}