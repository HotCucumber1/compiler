#include "SemanticAnalyzer.h"
#include "../ErrorHandler/ErrorHandler.h"

#include <ranges>

void SemanticAnalyzer::Analyze(const std::shared_ptr<AstNode>& root)
{
	m_symbolTable.clear();
	EnterScope();

	if (root)
	{
		root->Accept(*this);
	}
	ExitScope();
}

void SemanticAnalyzer::Visit(TerminalNode& node)
{
}

void SemanticAnalyzer::Visit(CommonNode& node)
{
	for (const auto& child : node.children)
	{
		if (child)
		{
			child->Accept(*this);
		}
	}
}

void SemanticAnalyzer::Visit(BinaryOpNode& node)
{
	if (node.left)
	{
		node.left->Accept(*this);
	}
	if (node.right)
	{
		node.right->Accept(*this);
	}

	if (node.left && node.right)
	{
		if (node.left->evaluatedType != node.right->evaluatedType)
		{
			throw CompilerError(node.location, "Semantic error: Cannot operate on different types!");
		}
		if (node.left->arrayDepth > 0 || node.right->arrayDepth > 0)
		{
			throw CompilerError(node.location, "Semantic error: Binary operations on arrays are not supported.");
		}

		if (node.op.value == "=="
			|| node.op.value == "!="
			|| node.op.value == "<"
			|| node.op.value == ">")
		{
			node.evaluatedType = DataType::BOOL;
		}
		else
		{
			node.evaluatedType = node.left->evaluatedType;
		}

		node.arrayDepth = 0;
		node.structName = "";
	}
}

void SemanticAnalyzer::Visit(UnaryOpNode& node)
{
	if (node.operand)
	{
		node.operand->Accept(*this);

		if (node.operand->arrayDepth > 0)
		{
			throw CompilerError(node.location, "Semantic error: Unary operations on arrays are not allowed.");
		}

		node.evaluatedType = node.operand->evaluatedType;
		node.arrayDepth = 0;
		node.structName = "";
	}
}

void SemanticAnalyzer::Visit(VarDeclNode& node)
{
	if (m_symbolTable.back().contains(node.varName))
	{
		throw CompilerError(node.location, "Semantic error: Variable already declared: " + node.varName);
	}

	if (node.expr)
	{
		node.expr->Accept(*this);

		if (node.expr->evaluatedType != DataType::UNKNOWN && node.expr->evaluatedType != node.varType)
		{
			throw CompilerError(node.location, "Semantic error: Type mismatch for " + node.varName);
		}
		if (node.expr->arrayDepth != node.arrayDepth)
		{
			throw CompilerError(node.location, "Semantic error: Array depth mismatch for " + node.varName);
		}
		if (node.expr->evaluatedType == DataType::INSTANCE
			&& node.expr->structName != node.structName)
		{
			throw CompilerError(node.location, "Semantic error: Struct type mismatch for " + node.varName);
		}
	}

	m_symbolTable.back()[node.varName] = {
		node.varType,
		node.arrayDepth,
		node.structName,
		node.isConst
	};
}

void SemanticAnalyzer::Visit(LiteralNode& node)
{
	node.arrayDepth = 0;
	node.structName = "";

	switch (node.token.type)
	{
	case TokenType::INT:
		node.evaluatedType = DataType::INT;
		break;
	case TokenType::FLOAT:
		node.evaluatedType = DataType::FLOAT;
		break;
	case TokenType::TRUE:
	case TokenType::FALSE:
		node.evaluatedType = DataType::BOOL;
		break;
	case TokenType::STRING:
		node.evaluatedType = DataType::STRING;
		break;
	default:
		node.evaluatedType = DataType::UNKNOWN;
		break;
	}
}

void SemanticAnalyzer::Visit(VarAccessNode& node)
{
	for (const auto& scope : std::ranges::reverse_view(m_symbolTable))
	{
		if (scope.contains(node.varName))
		{
			const auto& varInfo = scope.at(node.varName);
			node.evaluatedType = varInfo.type;
			node.arrayDepth = varInfo.arrayDepth;
			node.structName = varInfo.structName;
			return;
		}
	}
	throw CompilerError(node.location, "Semantic error: Undefined variable '" + node.varName + "'");
}

void SemanticAnalyzer::Visit(AssignmentNode& node)
{
	if (node.expr)
	{
		node.expr->Accept(*this);
	}
	bool found = false;
	SymbolInfo varInfo{};
	for (const auto& scope : std::ranges::reverse_view(m_symbolTable))
	{
		if (scope.contains(node.varName))
		{
			varInfo = scope.at(node.varName);
			found = true;
			break;
		}
	}
	if (!found)
	{
		throw CompilerError(node.location, "Semantic error: Assignment to undefined variable '" + node.varName + "'");
	}
	if (varInfo.isConst)
	{
		throw CompilerError(node.location, "Semantic error: Cannot assign to constant variable '" + node.varName + "'");
	}
	if (node.expr)
	{
		if (node.expr->evaluatedType != DataType::UNKNOWN && node.expr->evaluatedType != varInfo.type)
		{
			throw CompilerError(node.location, "Semantic error: Type mismatch in assignment to '" + node.varName + "'");
		}
		if (node.expr->arrayDepth != varInfo.arrayDepth)
		{
			throw CompilerError(node.location, "Semantic error: Array depth mismatch in assignment to '" + node.varName + "'");
		}
		if (node.expr->evaluatedType == DataType::INSTANCE && node.expr->structName != varInfo.structName)
		{
			throw CompilerError(node.location, "Semantic error: Struct type mismatch in assignment to '" + node.varName + "'");
		}
	}
}

void SemanticAnalyzer::Visit(IfNode& node)
{
	if (node.condition)
	{
		node.condition->Accept(*this);
	}

	EnterScope();
	if (node.thenBranch)
	{
		node.thenBranch->Accept(*this);
	}
	ExitScope();

	if (node.elseBranch)
	{
		EnterScope();
		node.elseBranch->Accept(*this);
		ExitScope();
	}
}

void SemanticAnalyzer::Visit(WhileNode& node)
{
	if (node.condition)
	{
		node.condition->Accept(*this);
	}

	EnterScope();
	if (node.body)
	{
		node.body->Accept(*this);
	}
	ExitScope();
}

void SemanticAnalyzer::Visit(ForNode& node)
{
	EnterScope();

	if (node.init)
	{
		node.init->Accept(*this);
	}
	if (node.condition)
	{
		node.condition->Accept(*this);
	}
	if (node.increment)
	{
		node.increment->Accept(*this);
	}
	if (node.body)
	{
		node.body->Accept(*this);
	}

	ExitScope();
}

void SemanticAnalyzer::Visit(CallNode& node)
{
	for (const auto& arg : node.args)
	{
		if (arg)
		{
			arg->Accept(*this);
		}
	}
	if (m_functions.contains(node.funcName))
	{
		const auto& funcInfo = m_functions.at(node.funcName);
		node.evaluatedType = funcInfo.type;
		node.arrayDepth = funcInfo.arrayDepth;
		node.structName = funcInfo.structName;
	}
	else
	{
		node.evaluatedType = DataType::UNKNOWN;
		node.arrayDepth = 0;
		node.structName = "";
	}
}

void SemanticAnalyzer::Visit(FunctionDeclNode& node)
{
	m_functions[node.name] = { node.returnType, node.arrayDepth, node.structName, false };

	const auto previousSignature = m_currentFunctionSignature;
	m_currentFunctionSignature = { node.returnType, node.arrayDepth, node.structName, false };

	EnterScope();

	for (const auto& paramNode : node.parameters)
	{
		if (paramNode)
		{
			m_symbolTable.back()[paramNode->varName] = {
				paramNode->varType,
				paramNode->arrayDepth,
				paramNode->structName,
				paramNode->isConst
			};
		}
	}

	if (node.body)
	{
		node.body->Accept(*this);
	}
	ExitScope();
	m_currentFunctionSignature = previousSignature;
}

void SemanticAnalyzer::Visit(StructDeclNode& node)
{
	for (const auto& field : node.fields)
	{
		if (field)
		{
			field->Accept(*this);
		}
	}
}

void SemanticAnalyzer::Visit(StructInitNode& node)
{
	for (const auto& arg : node.args)
	{
		if (arg)
		{
			arg->Accept(*this);
		}
	}

	node.evaluatedType = DataType::INSTANCE;
	node.arrayDepth = 0;
}

void SemanticAnalyzer::Visit(MemberAccessNode& node)
{
	if (node.object)
	{
		node.object->Accept(*this);
	}

	node.evaluatedType = DataType::UNKNOWN;
	node.arrayDepth = 0;
	node.structName = "";
}

void SemanticAnalyzer::Visit(MemberAssignmentNode& node)
{
	if (node.object)
	{
		node.object->Accept(*this);
	}
	if (node.expression)
	{
		node.expression->Accept(*this);
	}
}

void SemanticAnalyzer::Visit(ArrayLiteralNode& node)
{
	node.evaluatedType = DataType::UNKNOWN;
	node.arrayDepth = 1;
	node.structName = "";

	if (!node.elements.empty())
	{
		auto firstType = DataType::UNKNOWN;
		int firstDepth = 0;
		std::string firstName;
		bool first = true;

		for (const auto& elem : node.elements)
		{
			if (elem)
			{
				elem->Accept(*this);

				if (first)
				{
					firstType = elem->evaluatedType;
					firstDepth = elem->arrayDepth;
					firstName = elem->structName;
					first = false;
				}
				else if (elem->evaluatedType != DataType::UNKNOWN
					&& (elem->evaluatedType != firstType || elem->arrayDepth != firstDepth || elem->structName != firstName))
				{
					throw CompilerError(node.location, "Semantic error: All elements in array literal must have the same exact type.");
				}
			}
		}
		node.evaluatedType = firstType;
		node.arrayDepth = firstDepth + 1;
		node.structName = firstName;
	}
}

void SemanticAnalyzer::Visit(IndexAccessNode& node)
{
	if (node.target)
	{
		node.target->Accept(*this);
	}
	if (node.index)
	{
		node.index->Accept(*this);
	}

	if (node.target
		&& node.target->arrayDepth == 0
		&& node.target->evaluatedType != DataType::UNKNOWN)
	{
		throw CompilerError(node.location, "Semantic error: Cannot index a non-array target.");
	}
	if (node.index && node.index->evaluatedType != DataType::UNKNOWN && node.index->evaluatedType != DataType::INT)
	{
		throw CompilerError(node.location, "Semantic error: Array index must be an integer.");
	}

	if (node.target)
	{
		node.evaluatedType = node.target->evaluatedType;
		node.structName = node.target->structName;
		node.arrayDepth = node.target->arrayDepth > 0
			? node.target->arrayDepth - 1
			: 0;
	}
}

void SemanticAnalyzer::Visit(IndexAssignmentNode& node)
{
	if (node.target)
	{
		node.target->Accept(*this);
	}
	if (node.index)
	{
		node.index->Accept(*this);
	}
	if (node.value)
	{
		node.value->Accept(*this);
	}

	if (node.target && node.target->arrayDepth == 0 && node.target->evaluatedType != DataType::UNKNOWN)
	{
		throw CompilerError(node.location, "Semantic error: Cannot index a non-array target.");
	}
	if (node.index && node.index->evaluatedType != DataType::UNKNOWN && node.index->evaluatedType != DataType::INT)
	{
		throw CompilerError(node.location, "Semantic error: Array index must be an integer.");
	}

	if (node.target && node.value
		&& node.target->evaluatedType != DataType::UNKNOWN
		&& node.value->evaluatedType != DataType::UNKNOWN)
	{
		if (node.target->evaluatedType != node.value->evaluatedType
			|| node.target->structName != node.value->structName
			|| (node.target->arrayDepth - 1) != node.value->arrayDepth)
		{
			throw CompilerError(node.location, "Semantic error: Type mismatch in array assignment.");
		}
	}
}

void SemanticAnalyzer::Visit(ReturnNode& node)
{
	if (node.expr)
	{
		node.expr->Accept(*this);
		if (m_currentFunctionSignature.type != DataType::UNKNOWN)
		{
			if (node.expr->evaluatedType != m_currentFunctionSignature.type
				|| node.expr->arrayDepth != m_currentFunctionSignature.arrayDepth
				|| node.expr->structName != m_currentFunctionSignature.structName)
			{
				auto errorMsg = "Semantic error: Return type mismatch. Expected type " + DATA_TYPE_NAMES.at(m_currentFunctionSignature.type);
				if (m_currentFunctionSignature.arrayDepth > 0)
				{
					for (int i = 0; i < m_currentFunctionSignature.arrayDepth; ++i)
					{
						errorMsg += "[]";
					}
				}
				throw CompilerError(node.location, errorMsg);
			}
		}
		return;
	}
	if (m_currentFunctionSignature.type != DataType::UNKNOWN
		&& m_currentFunctionSignature.type != DataType::VOID)
	{
		throw CompilerError(node.location, "Semantic error: Empty return statement in a non-void function.");
	}
}

void SemanticAnalyzer::Visit(ImportNode& node)
{
}

void SemanticAnalyzer::EnterScope()
{
	m_symbolTable.emplace_back();
}

void SemanticAnalyzer::ExitScope()
{
	if (!m_symbolTable.empty())
	{
		m_symbolTable.pop_back();
	}
}
