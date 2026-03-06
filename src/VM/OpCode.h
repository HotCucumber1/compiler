#pragma once

enum class OpCode : uint8_t
{
	OP_CONSTANT = 0,

	OP_NEG,
	OP_ADD,
	OP_SUB,
	OP_MULTI,
	OP_DIVIDE,

	OP_RETURN = 255,
};
