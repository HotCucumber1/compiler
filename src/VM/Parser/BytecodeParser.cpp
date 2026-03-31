#include "BytecodeParser.h"
#include "../OpCode.h"

#include <iomanip>
#include <sstream>

constexpr char METKA = ':';

const std::string LOCALS = ".locals";
const std::string CONST = ".const";
const std::string CODE = ".code";

void PatchJumps(Chunk* chunk);
void ParseConstant(std::stringstream& ss, Chunk* chunk);
void ParseInstruction(std::stringstream& ss, Chunk* chunk);

void LocalsOp(std::stringstream& ss, Chunk* chunk, OpCode code);
void JmpOp(std::stringstream& ss, Chunk* chunk, OpCode code);

std::unique_ptr<Chunk> BytecodeParser::Parse(const std::string& source)
{
	auto chunk = std::make_unique<Chunk>();
	std::stringstream ss(source);
	std::string line;
	bool parsingConstants = false;
	bool parsingCode = false;

	while (std::getline(ss, line))
	{
		auto first = line.find_first_not_of(" \t\r");
		if (first == std::string::npos)
		{
			continue;
		}
		auto last = line.find_last_not_of(" \t\r");
		line = line.substr(first, (last - first + 1));

		if (line[0] == ';')
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
	PatchJumps(chunk.get());
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

	if (op.back() == METKA)
	{
		const auto label = op.substr(0, op.length() - 1);
		chunk->labels[label] = chunk->code.size();
		ss >> op;
	}

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
	else if (op == "jmp")
	{
		JmpOp(ss, chunk, OpCode::OP_JMP);
	}
	else if (op == "jmp_if_false")
	{
		JmpOp(ss, chunk, OpCode::OP_JMP_IF_FALSE);
	}
	else if (op == "create_arr")
	{
		int count;
		ss >> count;
		chunk->code.push_back(static_cast<uint8_t>(OpCode::OP_CREATE_ARRAY));
		chunk->code.push_back(static_cast<uint8_t>(count));
	}
	else if (op == "get_el")
	{
		chunk->code.push_back(static_cast<uint8_t>(OpCode::OP_GET_ELEMENT));
	}
	else if (op == "set_el")
	{
		chunk->code.push_back(static_cast<uint8_t>(OpCode::OP_SET_ELEMENT));
	}
	else if (op == "get_global")
	{
		std::string name;
		ss >> name;

		chunk->constants.emplace_back(MakeString(name));
		const auto index = static_cast<uint8_t>(chunk->constants.size() - 1);
		chunk->code.push_back(static_cast<uint8_t>(OpCode::OP_GET_GLOBAL));
		chunk->code.push_back(index);
	}
	else if (op == "set_global")
	{
		std::string name;
		ss >> name;

		chunk->constants.emplace_back(MakeString(name));
		const auto index = static_cast<uint8_t>(chunk->constants.size() - 1);
		chunk->code.push_back(static_cast<uint8_t>(OpCode::OP_SET_GLOBAL));
		chunk->code.push_back(index);
	}
}

void PatchJumps(Chunk* chunk)
{
	for (const auto& [pos, label] : chunk->pendingJumps)
	{
		const auto it = chunk->labels.find(label);
		if (it == chunk->labels.end())
		{
			throw std::runtime_error("Undefined label: " + label);
		}

		const auto targetAddr = it->second;
		if (targetAddr > 65535) // TODO max
		{
			throw std::runtime_error("Label address too large: " + label);
		}

		chunk->code[pos] = static_cast<uint8_t>((targetAddr >> 8) & 0xFF);
		chunk->code[pos + 1] = static_cast<uint8_t>(targetAddr & 0xFF);
	}
}

void LocalsOp(std::stringstream& ss, Chunk* chunk, OpCode code)
{
	int index;
	ss >> index;
	if (index >= chunk->localsCount)
	{
		throw std::runtime_error(
			"Wrong local variable index: " + std::to_string(index)
			+ "\nMax: " + std::to_string(chunk->localsCount));
	}
	chunk->code.push_back(static_cast<uint8_t>(code));
	chunk->code.push_back(static_cast<uint8_t>(index));
}

void JmpOp(std::stringstream& ss, Chunk* chunk, OpCode code)
{
	std::string label;
	ss >> label;

	const auto jmpPos = chunk->code.size();
	chunk->code.push_back(static_cast<uint8_t>(code));
	chunk->code.push_back(0);
	chunk->code.push_back(0);

	chunk->pendingJumps.emplace_back(jmpPos + 1, label);
}
