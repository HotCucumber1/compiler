#include "SemanticAnalyzer.h"

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
	node.left->Accept(*this);
	node.right->Accept(*this);

	if (node.left->evaluatedType != node.right->evaluatedType)
	{
		throw std::runtime_error("Semantic error: Cannot operate on different types!");
	}
	if (node.left->isArrayExpr || node.right->isArrayExpr)
	{
		throw std::runtime_error("Semantic error: Binary operations on arrays are not supported.");
	}
	if (node.op.value == "==" || node.op.value == "!=" || node.op.value == "<" || node.op.value == ">")
	{
		node.evaluatedType = DataType::BOOL;
	}
	else
	{
		node.evaluatedType = node.left->evaluatedType;
	}
	node.isArrayExpr = false;
}

void SemanticAnalyzer::Visit(UnaryOpNode& node)
{
	if (node.operand)
	{
		node.operand->Accept(*this);

		if (node.operand->isArrayExpr)
		{
			throw std::runtime_error("Semantic error: Unary operations on arrays are not allowed.");
		}

		node.evaluatedType = node.operand->evaluatedType;
		node.isArrayExpr = false;
	}
}

void SemanticAnalyzer::Visit(VarDeclNode& node)
{
	if (m_symbolTable.back().contains(node.varName))
	{
		throw std::runtime_error("Semantic error: Variable already declared: " + node.varName);
	}

	if (node.expr)
	{
		node.expr->Accept(*this);

		if (node.expr->evaluatedType != node.varType)
		{
			throw std::runtime_error("Semantic error: Type mismatch for " + node.varName);
		}
		if (node.expr->isArrayExpr != node.isArray)
		{
			throw std::runtime_error("Semantic error: Array assignment mismatch for " + node.varName);
		}
	}
	m_symbolTable.back()[node.varName] = {
		node.varType,
		node.isArray,
		node.isConst,
	};
}

void SemanticAnalyzer::Visit(LiteralNode& node)
{
	node.isArrayExpr = false;

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
			node.isArrayExpr = varInfo.isArray;
			return;
		}
	}
	throw std::runtime_error("Semantic error: Undefined variable '" + node.varName + "'");
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
		throw std::runtime_error("Semantic error: Assignment to undefined variable '" + node.varName + "'");
	}
	if (varInfo.isConst)
	{
		throw std::runtime_error("Semantic error: Cannot assign to constant variable '" + node.varName + "'");
	}
	if (node.expr)
	{
		if (node.expr->evaluatedType != DataType::UNKNOWN && node.expr->evaluatedType != varInfo.type)
		{
			throw std::runtime_error("Semantic error: Type mismatch in assignment to '" + node.varName + "'");
		}
		if (node.expr->isArrayExpr != varInfo.isArray)
		{
			throw std::runtime_error("Semantic error: Array/scalar mismatch in assignment to '" + node.varName + "'");
		}
	}
}

void SemanticAnalyzer::Visit(IfNode& node)
{
	if (node.condition)
	{
		node.condition->Accept(*this);
		if (node.condition->evaluatedType != DataType::BOOL)
		{
			// throw std::runtime_error("Semantic error: If condition must be boolean");
		}
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
		node.isArrayExpr = funcInfo.isArray;
	}
	else
	{
		node.evaluatedType = DataType::UNKNOWN;
		node.isArrayExpr = false;
	}
}

void SemanticAnalyzer::Visit(FunctionDeclNode& node)
{
	m_functions[node.name] = { node.returnType, node.isReturnArray, false };

	EnterScope();

	for (const auto& paramName : node.parameters)
	{
		m_symbolTable.back()[paramName] = { DataType::UNKNOWN, false, false };
	}
	if (node.body)
	{
		node.body->Accept(*this);
	}

	ExitScope();
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
	node.isArrayExpr = false;
}

void SemanticAnalyzer::Visit(MemberAccessNode& node)
{
	if (node.object)
	{
		node.object->Accept(*this);
	}

	node.evaluatedType = DataType::UNKNOWN;
	node.isArrayExpr = false;
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
	node.isArrayExpr = true;
	node.evaluatedType = DataType::UNKNOWN;

	if (!node.elements.empty())
	{
		auto firstType = DataType::UNKNOWN;
		bool first = true;

		for (const auto& elem : node.elements)
		{
			if (elem)
			{
				elem->Accept(*this);

				if (first)
				{
					firstType = elem->evaluatedType;
					first = false;
				}
				else if (elem->evaluatedType != DataType::UNKNOWN && elem->evaluatedType != firstType)
				{
					throw std::runtime_error("Semantic error: All elements in array literal must have the same type.");
				}
			}
		}
		node.evaluatedType = firstType;
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

	if (node.target && !node.target->isArrayExpr)
	{
		throw std::runtime_error("Semantic error: Cannot index a non-array target.");
	}
	if (node.index && node.index->evaluatedType != DataType::UNKNOWN && node.index->evaluatedType != DataType::INT)
	{
		throw std::runtime_error("Semantic error: Array index must be an integer.");
	}

	node.isArrayExpr = false;
	node.evaluatedType = node.target ? node.target->evaluatedType : DataType::UNKNOWN;
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

	if (node.target && !node.target->isArrayExpr)
	{
		throw std::runtime_error("Semantic error: Cannot index a non-array target.");
	}
	if (node.index && node.index->evaluatedType != DataType::UNKNOWN && node.index->evaluatedType != DataType::INT)
	{
		throw std::runtime_error("Semantic error: Array index must be an integer.");
	}

	if (node.target && node.value && node.target->evaluatedType != DataType::UNKNOWN && node.value->evaluatedType != DataType::UNKNOWN && node.target->evaluatedType != node.value->evaluatedType)
	{
		throw std::runtime_error("Semantic error: Type mismatch in array assignment.");
	}
}

void SemanticAnalyzer::Visit(ReturnNode& node)
{
	if (node.expr)
	{
		node.expr->Accept(*this);
	}
	// Когда в FunctionDeclNode будет добавлен возврат типа, здесь его сверять с node.expr->evaluatedType
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
