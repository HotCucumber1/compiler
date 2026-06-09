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
	std::cout << "[Call] " << node.funcName << "\n";

	m_depth++;
	PrintIndent();
	std::cout << "Arguments:\n";

	m_depth++;
	if (node.args.empty())
	{
		PrintIndent();
		std::cout << "(none)\n";
	}
	else
	{
		for (const auto& arg : node.args)
		{
			if (arg)
			{
				arg->Accept(*this);
			}
		}
	}
	m_depth--;
	m_depth--;
}

void AstPrinter::Visit(FunctionDeclNode& node)
{
	PrintIndent();
	std::cout << "[FunctionDecl] " << node.name;
	if (node.arrayDepth > 0)
	{
		std::cout << " (Return Array depth: " << node.arrayDepth << ")";
	}
	std::cout << "\n";

	m_depth++;

	PrintIndent();
	std::cout << "Parameters:\n";
	m_depth++;
	if (node.parameters.empty())
	{
		PrintIndent();
		std::cout << "(none)\n";
	}
	else
	{
		for (const auto& param : node.parameters)
		{
			if (param)
			{
				param->Accept(*this);
			}
		}
	}
	m_depth--;

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

void AstPrinter::Visit(StructDeclNode& node)
{
	PrintIndent();
	std::cout << "[StructDecl] " << node.name << "\n";

	m_depth++;
	PrintIndent();
	std::cout << "Fields:\n";

	m_depth++;
	if (node.fields.empty())
	{
		PrintIndent();
		std::cout << "(none)\n";
	}
	else
	{
		for (const auto& field : node.fields)
		{
			if (field)
			{
				field->Accept(*this);
			}
		}
	}
	m_depth--;
	m_depth--;
}

void AstPrinter::Visit(StructInitNode& node)
{
	PrintIndent();
	std::cout << "[StructInit] " << node.structName << "\n";

	m_depth++;
	PrintIndent();
	std::cout << "Arguments:\n";

	m_depth++;
	if (node.args.empty())
	{
		PrintIndent();
		std::cout << "(none)\n";
	}
	else
	{
		for (const auto& arg : node.args)
		{
			if (arg)
			{
				arg->Accept(*this);
			}
		}
	}
	m_depth--;
	m_depth--;
}

void AstPrinter::Visit(MemberAccessNode& node)
{
	PrintIndent();
	std::cout << "[MemberAccess] ." << node.fieldName << "\n";

	m_depth++;
	PrintIndent();
	std::cout << "Object:\n";

	m_depth++;
	if (node.object)
	{
		node.object->Accept(*this);
	}
	m_depth--;
	m_depth--;
}

void AstPrinter::Visit(MemberAssignmentNode& node)
{
	PrintIndent();
	std::cout << "[MemberAssignment] ." << node.fieldName << " =\n";

	m_depth++;

	PrintIndent();
	std::cout << "Object:\n";
	m_depth++;
	if (node.object)
	{
		node.object->Accept(*this);
	}
	m_depth--;

	PrintIndent();
	std::cout << "Value:\n";
	m_depth++;
	if (node.expression)
	{
		node.expression->Accept(*this);
	}
	m_depth--;

	m_depth--;
}

void AstPrinter::Visit(ArrayLiteralNode& node)
{
	PrintIndent();
	std::cout << "[ArrayLiteral]\n";

	m_depth++;
	if (node.elements.empty())
	{
		PrintIndent();
		std::cout << "(empty)\n";
	}
	else
	{
		for (const auto& elem : node.elements)
		{
			if (elem)
			{
				elem->Accept(*this);
			}
		}
	}
	m_depth--;
}

void AstPrinter::Visit(IndexAccessNode& node)
{
	PrintIndent();
	std::cout << "[IndexAccess]\n";

	m_depth++;

	PrintIndent();
	std::cout << "Target:\n";
	m_depth++;
	if (node.target)
	{
		node.target->Accept(*this);
	}
	m_depth--;

	PrintIndent();
	std::cout << "Index:\n";
	m_depth++;
	if (node.index)
	{
		node.index->Accept(*this);
	}
	m_depth--;
	m_depth--;
}

void AstPrinter::Visit(IndexAssignmentNode& node)
{
	PrintIndent();
	std::cout << "[IndexAssignment] =\n";

	m_depth++;

	PrintIndent();
	std::cout << "Target:\n";
	m_depth++;
	if (node.target)
	{
		node.target->Accept(*this);
	}
	m_depth--;

	PrintIndent();
	std::cout << "Index:\n";
	m_depth++;
	if (node.index)
	{
		node.index->Accept(*this);
	}
	m_depth--;

	PrintIndent();
	std::cout << "Value:\n";
	m_depth++;
	if (node.value)
	{
		node.value->Accept(*this);
	}
	m_depth--;
	m_depth--;
}

void AstPrinter::Visit(ReturnNode& node)
{
	PrintIndent();
	std::cout << "[Return]" << std::endl;

	if (node.expr)
	{
		m_depth++;
		node.expr->Accept(*this);
		m_depth--;
	}
}

void AstPrinter::Visit(ImportNode& node)
{
	PrintIndent();
	std::cout << "[Import] " << node.filePath << "\n";
}

void AstPrinter::Visit(SchemaFuncNode& node)
{
	PrintIndent();
	std::cout << "[SchemaFunc] " << node.name << " -> " << node.returnState << "\n";

	m_depth++;
	PrintIndent();
	std::cout << "Parameters:\n";

	m_depth++;
	if (node.parameters.empty())
	{
		PrintIndent();
		std::cout << "(none)\n";
	}
	else
	{
		for (const auto& param : node.parameters)
		{
			if (param)
			{
				param->Accept(*this);
			}
		}
	}
	m_depth--;
	m_depth--;
}

void AstPrinter::Visit(SchemaStateNode& node)
{
	PrintIndent();
	std::cout << "[SchemaState] " << node.name << "\n";

	m_depth++;

	PrintIndent();
	std::cout << "Fields:\n";
	m_depth++;
	if (node.fields.empty())
	{
		PrintIndent();
		std::cout << "(none)\n";
	}
	else
	{
		for (const auto& field : node.fields)
		{
			if (field)
			{
				field->Accept(*this);
			}
		}
	}
	m_depth--;

	PrintIndent();
	std::cout << "Transitions:\n";
	m_depth++;
	if (node.methods.empty())
	{
		PrintIndent();
		std::cout << "(none)\n";
	}
	else
	{
		for (const auto& method : node.methods)
		{
			if (method)
			{
				method->Accept(*this);
			}
		}
	}
	m_depth--;

	m_depth--;
}

void AstPrinter::Visit(SchemaDeclNode& node)
{
	PrintIndent();
	std::cout << "[SchemaDecl] " << node.name << "\n";

	m_depth++;
	PrintIndent();
	std::cout << "States:\n";

	m_depth++;
	if (node.states.empty())
	{
		PrintIndent();
		std::cout << "(none)\n";
	}
	else
	{
		for (const auto& state : node.states)
		{
			if (state)
			{
				state->Accept(*this);
			}
		}
	}
	m_depth--;
	m_depth--;
}

void AstPrinter::Visit(ImplDeclNode& node)
{
	PrintIndent();
	std::cout << "[ImplDecl] Schema: " << node.schemaName << "<" << node.stateName << ">\n";

	m_depth++;

	PrintIndent();
	std::cout << "Methods:\n";
	m_depth++;

	if (node.methods.empty())
	{
		PrintIndent();
		std::cout << "(none)\n";
	}
	else
	{
		for (const auto& method : node.methods)
		{
			if (method)
			{
				method->Accept(*this);
			}
		}
	}
	m_depth--;
	m_depth--;
}

void AstPrinter::Visit(MethodDeclNode& node)
{
	PrintIndent();
	std::cout << "[MethodDecl] " << (node.isPublic ? "public" : "private")
			  << " " << node.name << " -> ";

	if (node.targetState)
	{
		std::cout << node.targetState->schemaName << "<" << node.targetState->stateName << ">\n";
	}
	else
	{
		std::cout << "UNKNOWN\n";
	}

	m_depth++;

	PrintIndent();
	std::cout << "Parameters:\n";
	m_depth++;
	if (node.parameters.empty())
	{
		PrintIndent();
		std::cout << "(none)\n";
	}
	else
	{
		for (const auto& param : node.parameters)
		{
			if (param)
			{
				param->Accept(*this);
			}
		}
	}
	m_depth--;

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

void AstPrinter::Visit(ImplStateNode& node)
{
	PrintIndent();
	std::cout << "[ImplState] " << node.schemaName << "<" << node.stateName << ">\n";
}

void AstPrinter::Visit(TsopInitNode& node)
{
	PrintIndent();
	std::cout << "[TsopInit] new " << node.schemaName << "<" << node.stateName << ">\n";

	m_depth++;
	PrintIndent();
	std::cout << "Arguments:\n";

	m_depth++;
	if (node.args.empty())
	{
		PrintIndent();
		std::cout << "(none)\n";
	}
	else
	{
		for (const auto& arg : node.args)
		{
			if (arg)
			{
				arg->Accept(*this);
			}
		}
	}
	m_depth--;
	m_depth--;
}

void AstPrinter::Visit(MethodCallNode& node)
{
	PrintIndent();
	std::cout << "[MethodCall] ." << node.methodName << "()\n";

	m_depth++;

	PrintIndent();
	std::cout << "Object:\n";
	m_depth++;
	if (node.object)
	{
		node.object->Accept(*this);
	}
	m_depth--;

	PrintIndent();
	std::cout << "Arguments:\n";
	m_depth++;
	if (node.args.empty())
	{
		PrintIndent();
		std::cout << "(none)\n";
	}
	else
	{
		for (const auto& arg : node.args)
		{
			if (arg)
			{
				arg->Accept(*this);
			}
		}
	}
	m_depth--;
	m_depth--;
}

void AstPrinter::Visit(EnumDeclNode& node)
{
	PrintIndent();
	std::cout << "[EnumDecl] " << node.name << " : (backing type)\n";

	m_depth++;
	for (const auto& c : node.cases)
	{
		if (c)
		{
			c->Accept(*this);
		}
	}
	m_depth--;
}

void AstPrinter::Visit(EnumCaseNode& node)
{
	PrintIndent();
	std::cout << "├── case " << node.name << " = \n";

	m_depth++;
	if (node.value)
	{
		node.value->Accept(*this);
	}
	m_depth--;
}

void AstPrinter::Visit(MatchCaseNode& node)
{
	PrintIndent();
	std::cout << "├── [MatchCase] =>\n";

	m_depth++;
	if (node.pattern)
	{
		node.pattern->Accept(*this);
	}
	if (node.body)
	{
		node.body->Accept(*this);
	}
	m_depth--;
}

void AstPrinter::Visit(MatchNode& node)
{
	PrintIndent();
	std::cout << "[MatchExpr]\n";

	m_depth++;
	if (node.expr)
	{
		node.expr->Accept(*this);
	}
	for (const auto& matchCase : node.cases)
	{
		if (matchCase)
		{
			matchCase->Accept(*this);
		}
	}
	m_depth--;
}

void AstPrinter::Visit(BreakNode& node)
{
	PrintIndent();
	std::cout << "[BreakNode]" << std::endl;
}

void AstPrinter::Visit(ContinueNode& node)
{
	PrintIndent();
	std::cout << "[ContinueNode]" << std::endl;
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
