#include "AstPrinter.h"

std::string GetAstNodeTypeName(AstNodeType type);

void AstPrinter::PrintAst(const std::shared_ptr<AstNode>& root)
{
	m_depth = 0;
	if (root)
	{
		std::cout << "--- AST DUMP ---" << std::endl;
		root->Accept(*this);
		std::cout << "----------------" << std::endl;
	}
	else
	{
		std::cout << "AST is empty (nullptr)." << std::endl;
	}
}

void AstPrinter::Visit(TerminalNode& node)
{
	PrintIndent();
	std::cout << "[Terminal] " << node.token.value << "\n";
}

void AstPrinter::Visit(CommonNode& node)
{
	PrintIndent();
	std::cout << "[Common: " << GetAstNodeTypeName(node.type) << "]\n";

	m_depth++;
	for (const auto& child : node.children)
	{
		if (child)
		{
			child->Accept(*this);
		}
	}
	m_depth--;
}

void AstPrinter::Visit(BinaryOpNode& node)
{
	PrintIndent();
	std::cout << "[BinaryOp] " << node.op.value << "\n";

	m_depth++;
	if (node.left)
	{
		node.left->Accept(*this);
	}
	if (node.right)
	{
		node.right->Accept(*this);
	}
	m_depth--;
}

void AstPrinter::Visit(UnaryOpNode& node)
{
	PrintIndent();
	std::cout << "[UnaryOp] " << node.op.value << "\n";

	m_depth++;
	if (node.operand)
	{
		node.operand->Accept(*this);
	}
	m_depth--;
}

void AstPrinter::Visit(VarDeclNode& node)
{
	PrintIndent();
	std::cout << "[" << (node.isConst ? "ConstDecl" : "VarDecl") << "] " << node.varName << "\n";

	if (node.expr)
	{
		m_depth++;
		node.expr->Accept(*this);
		m_depth--;
	}
}

void AstPrinter::Visit(LiteralNode& node)
{
	PrintIndent();
	std::cout << "[Literal] " << node.token.value << "\n";
}

void AstPrinter::Visit(VarAccessNode& node)
{
	PrintIndent();
	std::cout << "[VarAccess] " << node.varName << "\n";
}

void AstPrinter::Visit(AssignmentNode& node)
{
	PrintIndent();
	std::cout << "[Assignment] " << node.varName << " =\n";

	m_depth++;
	if (node.expr)
	{
		node.expr->Accept(*this);
	}
	m_depth--;
}

void AstPrinter::Visit(IfNode& node)
{
	PrintIndent();
	std::cout << "[IfStatement]\n";

	m_depth++;
	PrintIndent();
	std::cout << "Condition:\n";
	m_depth++;
	if (node.condition)
	{
		node.condition->Accept(*this);
	}
	m_depth--;

	PrintIndent();
	std::cout << "Then:\n";
	m_depth++;
	if (node.thenBranch)
	{
		node.thenBranch->Accept(*this);
	}
	m_depth--;

	if (node.elseBranch)
	{
		PrintIndent();
		std::cout << "Else:\n";
		m_depth++;
		node.elseBranch->Accept(*this);
		m_depth--;
	}
	m_depth--;
}

void AstPrinter::Visit(WhileNode& node)
{
	PrintIndent();
	std::cout << "[WhileStatement]\n";

	m_depth++;
	PrintIndent();
	std::cout << "Condition:\n";
	m_depth++;
	if (node.condition)
	{
		node.condition->Accept(*this);
	}
	m_depth--;

	PrintIndent();
	std::cout << "Body:\n";
	m_depth++;
	if (node.body)
	{
		node.body->Accept(*this);
	}
	m_depth--;
	m_depth--;
}

void AstPrinter::Visit(ForNode& node)
{
	PrintIndent();
	std::cout << "[ForStatement]\n";

	m_depth++;
	if (node.init)
	{
		PrintIndent();
		std::cout << "Init:\n";
		m_depth++;
		node.init->Accept(*this);
		m_depth--;
	}
	if (node.condition)
	{
		PrintIndent();
		std::cout << "Condition:\n";
		m_depth++;
		node.condition->Accept(*this);
		m_depth--;
	}
	if (node.increment)
	{
		PrintIndent();
		std::cout << "Increment:\n";
		m_depth++;
		node.increment->Accept(*this);
		m_depth--;
	}
	if (node.body)
	{
		PrintIndent();
		std::cout << "Body:\n";
		m_depth++;
		node.body->Accept(*this);
		m_depth--;
	}
	m_depth--;
}

void AstPrinter::Visit(CallNode& node)
{
	PrintIndent();
	std::cout << "[Call] " << node.funcName << "()\n";

	if (node.args)
	{
		m_depth++;
		PrintIndent();
		std::cout << "Args:\n";
		m_depth++;
		node.args->Accept(*this);
		m_depth--;
		m_depth--;
	}
}

void AstPrinter::PrintIndent() const
{
	std::cout << std::string(m_depth * 2, ' ');
}

std::string GetAstNodeTypeName(const AstNodeType type)
{
	switch (type)
	{
	case AstNodeType::UNDEFINED:
		return "UNDEFINED";
	case AstNodeType::PROGRAM:
		return "PROGRAM";
	case AstNodeType::BLOCK:
		return "BLOCK";
	case AstNodeType::EXPRESSION:
		return "EXPRESSION";
	default:
		return "UNKNOWN_AST_EXPRESSION";
	}
}
