#pragma once

enum class OpCode : uint8_t
{
	OP_CONSTANT = 0,

	OP_NEG,
	OP_ADD,
	OP_SUB,
	OP_MULTI,
	OP_DIV,
	OP_MOD,

	OP_SET_LOCAL_VAR,
	OP_GET_LOCAL_VAR,

	OP_RETURN = 255,
};
