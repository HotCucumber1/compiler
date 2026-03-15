#include "BytecodeParser.h"
#include "../OpCode.h"

#include <iomanip>
#include <sstream>

const std::string LOCALS = ".locals";
const std::string CONST = ".const";
const std::string CODE = ".code";

void ParseConstant(std::stringstream& ss, Chunk* chunk);
void ParseInstruction(std::stringstream& ss, Chunk* chunk);

void LocalsOp(std::stringstream& ss, Chunk* chunk, OpCode code);

std::unique_ptr<Chunk> BytecodeParser::Parse(const std::string& source)
{
	auto chunk = std::make_unique<Chunk>();
	std::stringstream ss(source);
	std::string line;
	bool parsingConstants = false;
	bool parsingCode = false;

	while (std::getline(ss, line))
	{
		if (line.empty() || line[0] == ';')
		{
			continue;
		}
		if (line.starts_with(LOCALS))
		{
			chunk->localsCount = std::stoi(line.substr(LOCALS.length()));
			continue;
		}

		if (line == CONST)
		{
			parsingConstants = true;
			parsingCode = false;
			continue;
		}
		if (line == CODE)
		{
			parsingConstants = false;
			parsingCode = true;
			continue;
		}

		std::stringstream lineStream(line);
		if (parsingConstants)
		{
			ParseConstant(lineStream, chunk.get());
		}
		else if (parsingCode)
		{
			ParseInstruction(lineStream, chunk.get());
		}
	}

	return chunk;
}

void ParseConstant(std::stringstream& ss, Chunk* chunk)
{
	std::string type;
	ss >> type;

	if (type == "int")
	{
		long long val;
		ss >> val;
		chunk->constants.emplace_back(val);
	}
	else if (type == "double")
	{
		double val;
		ss >> val;
		chunk->constants.emplace_back(val);
	}
	else if (type == "bool")
	{
		std::string val;
		ss >> val;
		chunk->constants.emplace_back(val == "true");
	}
	else if (type == "string")
	{
		std::string val;
		ss >> std::quoted(val);
		chunk->constants.emplace_back(MakeString(val));
	}
}

void ParseInstruction(std::stringstream& ss, Chunk* chunk)
{
	std::string op;
	ss >> op;

	if (op == "push")
	{
		int index;
		ss >> index;
		if (index >= chunk->constants.size())
		{
			throw std::invalid_argument(
				"Wrong const index: " + std::to_string(index) + "\nMax: " + std::to_string(chunk->constants.size() - 1));
		}
		chunk->code.push_back(static_cast<uint8_t>(OpCode::OP_CONSTANT));
		chunk->code.push_back(static_cast<uint8_t>(index));
	}
	else if (op == "return")
	{
		chunk->code.push_back(static_cast<uint8_t>(OpCode::OP_RETURN));
	}
	else if (op == "add")
	{
		chunk->code.push_back(static_cast<uint8_t>(OpCode::OP_ADD));
	}
	else if (op == "sub")
	{
		chunk->code.push_back(static_cast<uint8_t>(OpCode::OP_SUB));
	}
	else if (op == "multi")
	{
		chunk->code.push_back(static_cast<uint8_t>(OpCode::OP_MULTI));
	}
	else if (op == "div")
	{
		chunk->code.push_back(static_cast<uint8_t>(OpCode::OP_DIV));
	}
	else if (op == "mod")
	{
		chunk->code.push_back(static_cast<uint8_t>(OpCode::OP_MOD));
	}
	else if (op == "neg")
	{
		chunk->code.push_back(static_cast<uint8_t>(OpCode::OP_NEG));
	}
	else if (op == "set_local_var")
	{
		LocalsOp(ss, chunk, OpCode::OP_SET_LOCAL_VAR);
	}
	else if (op == "get_local_var")
	{
		LocalsOp(ss, chunk, OpCode::OP_GET_LOCAL_VAR);
	}
}

void LocalsOp(std::stringstream& ss, Chunk* chunk, OpCode code)
{
	int index;
	ss >> index;
	if (index >= chunk->localsCount)
	{
		throw std::invalid_argument(
			"Wrong local variable index: " + std::to_string(index)
			+ "\nMax: " + std::to_string(chunk->localsCount));
	}
	chunk->code.push_back(static_cast<uint8_t>(code));
	chunk->code.push_back(static_cast<uint8_t>(index));
}
