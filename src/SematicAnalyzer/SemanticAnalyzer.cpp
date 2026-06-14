#include "SemanticAnalyzer.h"
#include "../ErrorHandler/ErrorHandler.h"

#include <fstream>
#include <ranges>
#include <unordered_set>

bool IsTsopMatch(const std::string& left, const std::string& right);

void SemanticAnalyzer::Analyze(const std::shared_ptr<AstNode>& root)
{
	m_symbolTable.clear();

	RegisterNativeLib();

	EnterScope();
	AcceptNode(root);
	ExitScope();
}

void SemanticAnalyzer::Visit(TerminalNode& node)
{
}

void SemanticAnalyzer::Visit(CommonNode& node)
{
	const bool isBlock = (node.type == AstNodeType::BLOCK);

	if (isBlock)
	{
		EnterScope();
	}
	for (const auto& child : node.children)
	{
		AcceptNode(child);
	}
	if (isBlock)
	{
		ExitScope();
	}
}

void SemanticAnalyzer::Visit(BinaryOpNode& node)
{
	AcceptNode(node.left);
	AcceptNode(node.right);

	if (node.left && node.right)
	{
		if (node.op.value == "/" || node.op.value == "%")
		{
			if (const auto literalRight = std::dynamic_pointer_cast<LiteralNode>(node.right))
			{
				bool isZero = false;
				try
				{
					isZero = std::stod(literalRight->token.value) == 0.0;
				}
				catch (const std::exception&)
				{
				}
				if (isZero)
				{
					throw SemanticError(node.location, "Division by zero.");
				}
			}
		}

		const auto leftType = node.left->evaluatedType;
		const auto rightType = node.right->evaluatedType;

		const bool isNumericMix = (leftType == DataType::INT && rightType == DataType::FLOAT)
			|| (leftType == DataType::FLOAT && rightType == DataType::INT);

		if (leftType != rightType && !isNumericMix)
		{
			throw SemanticError(node.location, "Cannot operate on different types!");
		}
		if (node.left->arrayDepth > 0 || node.right->arrayDepth > 0)
		{
			throw SemanticError(node.location, "Binary operations on arrays are not supported.");
		}

		if (node.op.value == "=="
			|| node.op.value == "!="
			|| node.op.value == "<"
			|| node.op.value == ">")
		{
			node.evaluatedType = DataType::BOOL;
		}
		else if (node.op.value == "/" || isNumericMix)
		{
			node.evaluatedType = DataType::FLOAT;
		}
		else
		{
			node.evaluatedType = leftType;
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
			throw SemanticError(node.location, "Unary operations on arrays are not allowed.");
		}

		if (node.op.type == TokenType::MOVE)
		{
			node.evaluatedType = node.operand->evaluatedType;
			node.structName = node.operand->structName;
			node.arrayDepth = node.operand->arrayDepth;

			if (const auto varAccess = std::dynamic_pointer_cast<VarAccessNode>(node.operand))
			{
				bool found = false;
				for (auto& scope : std::ranges::reverse_view(m_symbolTable))
				{
					if (scope.contains(varAccess->varName))
					{
						auto& varInfo = scope.at(varAccess->varName);
						if (varInfo.isMoved)
						{
							throw SemanticError(node.location, "Variable '" + varAccess->varName + "' was already moved!");
						}
						varInfo.isMoved = true;
						found = true;
						break;
					}
				}
				if (!found)
				{
					throw SemanticError(node.location, "Cannot move undefined variable '" + varAccess->varName + "'");
				}
				return;
			}
			throw SemanticError(node.location, "The 'move' operator can only be applied to variables.");
		}
		node.evaluatedType = node.operand->evaluatedType;
		node.arrayDepth = 0;
		node.structName = "";
	}
}

void SemanticAnalyzer::Visit(VarDeclNode& node)
{
	ValidateTypeExists(node.varType, node.structName, node.location);
	if (m_symbolTable.back().contains(node.varName))
	{
		throw SemanticError(node.location, "Variable already declared: " + node.varName);
	}

	if (node.expr)
	{
		node.expr->Accept(*this);

		if (node.expr->evaluatedType != DataType::UNKNOWN && node.expr->evaluatedType != node.varType)
		{
			if (!(node.varType == DataType::FLOAT && node.expr->evaluatedType == DataType::INT))
			{
				throw SemanticError(node.location, "Type mismatch for " + node.varName);
			}
		}
		if (node.expr->arrayDepth != node.arrayDepth)
		{
			throw SemanticError(node.location, "Array depth mismatch for " + node.varName);
		}
		if (node.varType == DataType::INSTANCE && node.expr->evaluatedType == DataType::INSTANCE)
		{
			if (!IsTsopMatch(node.structName, node.expr->structName))
			{
				throw SemanticError(node.location, "Struct type mismatch for " + node.varName);
			}
			node.structName = node.expr->structName;
		}
		if (m_nodeTypes.contains(node.expr.get()))
		{
			m_tsopVars.back()[node.varName] = m_nodeTypes[node.expr.get()];
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
	if (m_enums.contains(node.varName))
	{
		node.isEnumName = true;
		node.evaluatedType = DataType::UNKNOWN;
		return;
	}

	auto itSym = m_symbolTable.rbegin();
	auto itTsop = m_tsopVars.rbegin();

	for (; itSym != m_symbolTable.rend(); ++itSym, ++itTsop)
	{
		if (itSym->contains(node.varName))
		{
			const auto& varInfo = itSym->at(node.varName);
			if (varInfo.isMoved)
			{
				throw SemanticError(node.location, "Use of moved variable '" + node.varName + "'. Ownership was transferred.");
			}

			node.evaluatedType = varInfo.type;
			node.arrayDepth = varInfo.arrayDepth;
			node.structName = varInfo.structName;

			if (itTsop->contains(node.varName))
			{
				m_nodeTypes[&node] = itTsop->at(node.varName);
			}
			return;
		}
	}
	if (m_functions.contains(node.varName))
	{
		node.evaluatedType = DataType::FUNC;
		node.arrayDepth = 0;
		node.structName = "";
		return;
	}
	throw SemanticError(node.location, "Undefined variable '" + node.varName + "'");
}

void SemanticAnalyzer::Visit(AssignmentNode& node)
{
	AcceptNode(node.expr);

	SymbolInfo* varInfoPtr = nullptr;

	for (auto& scope : std::ranges::reverse_view(m_symbolTable))
	{
		if (scope.contains(node.varName))
		{
			varInfoPtr = &scope.at(node.varName);
			break;
		}
	}

	if (!varInfoPtr)
	{
		throw SemanticError(node.location, "Assignment to undefined variable '" + node.varName + "'");
	}
	if (varInfoPtr->isConst)
	{
		throw SemanticError(node.location, "Cannot assign to constant variable '" + node.varName + "'");
	}
	if (varInfoPtr->isMoved)
	{
		throw SemanticError(node.location, " Cannot reassign to moved variable '" + node.varName + "'. Ownership was permanently transferred.");
	}
	if (!node.expr)
	{
		return;
	}
	if (varInfoPtr->type == DataType::INSTANCE && node.expr->evaluatedType == DataType::INSTANCE)
	{
		if (!IsTsopMatch(varInfoPtr->structName, node.expr->structName))
		{
			throw SemanticError(node.location, "Struct type mismatch in assignment to '" + node.varName + "'");
		}
		varInfoPtr->structName = node.expr->structName;
	}
	else if (node.expr->evaluatedType != DataType::UNKNOWN && node.expr->evaluatedType != varInfoPtr->type)
	{
		if (!(varInfoPtr->type == DataType::FLOAT && node.expr->evaluatedType == DataType::INT))
		{
			throw SemanticError(node.location, "Type mismatch in assignment to '" + node.varName + "'");
		}
	}

	if (node.expr->arrayDepth != varInfoPtr->arrayDepth)
	{
		throw SemanticError(node.location, "Array depth mismatch in assignment to '" + node.varName + "'");
	}
	node.evaluatedType = varInfoPtr->type;
	node.structName = varInfoPtr->structName;
	node.arrayDepth = varInfoPtr->arrayDepth;

	if (m_nodeTypes.contains(node.expr.get()))
	{
		auto itSym = m_symbolTable.rbegin();
		auto itTsop = m_tsopVars.rbegin();

		for (; itSym != m_symbolTable.rend(); ++itSym, ++itTsop)
		{
			if (itSym->contains(node.varName))
			{
				(*itTsop)[node.varName] = m_nodeTypes[node.expr.get()];
				break;
			}
		}
	}
}

void SemanticAnalyzer::Visit(IfNode& node)
{
	AcceptNode(node.condition);

	auto symTableBefore = m_symbolTable;
	auto tsopBefore = m_tsopVars;

	EnterScope();
	AcceptNode(node.thenBranch);
	ExitScope();

	auto symTableAfterThen = m_symbolTable;
	auto tsopAfterThen = m_tsopVars;

	m_symbolTable = symTableBefore;
	m_tsopVars = tsopBefore;

	if (node.elseBranch)
	{
		EnterScope();
		AcceptNode(node.elseBranch);
		ExitScope();

		const auto symTableAfterElse = m_symbolTable;
		const auto tsopAfterElse = m_tsopVars;

		for (size_t i = 0; i < tsopAfterThen.size(); ++i)
		{
			for (const auto& [varName, tsopType] : tsopAfterThen[i])
			{
				if (tsopAfterElse[i].contains(varName))
				{
					const auto& elseTsopType = tsopAfterElse[i].at(varName);
					if (tsopType.schema != elseTsopType.schema || tsopType.state != elseTsopType.state)
					{
						throw SemanticError(node.location,
							"Typestate mismatch for variable '" + varName + "' after if-else branches. 'then' branch leaves it in state '" + tsopType.state + "', but 'else' branch leaves it in '" + elseTsopType.state + "'. Both branches must result in the exact same state.");
					}
				}
			}
		}
		for (size_t i = 0; i < symTableAfterThen.size(); ++i)
		{
			for (const auto& [varName, symInfo] : symTableAfterThen[i])
			{
				if (symTableAfterElse[i].contains(varName))
				{
					const auto& elseSymInfo = symTableAfterElse[i].at(varName);
					if (symInfo.structName != elseSymInfo.structName)
					{
						throw SemanticError(node.location, "Struct state mismatch for variable '" + varName + "' after if-else branches.");
					}
					if (symInfo.isMoved != elseSymInfo.isMoved)
					{
						throw SemanticError(node.location, "Move semantics mismatch for variable '" + varName + "'. It was moved (ownership transferred) in one branch but not the other.");
					}
				}
			}
		}

		m_symbolTable = symTableAfterElse;
		m_tsopVars = tsopAfterElse;
	}
	else
	{
		for (size_t i = 0; i < tsopBefore.size(); ++i)
		{
			for (const auto& [varName, tsopTypeBefore] : tsopBefore[i])
			{
				if (tsopAfterThen[i].contains(varName))
				{
					const auto& [schema, state] = tsopAfterThen[i].at(varName);
					if (tsopTypeBefore.schema != schema || tsopTypeBefore.state != state)
					{
						throw SemanticError(node.location,
							"Typestate mismatch for variable '" + varName + "'. An 'if' statement without an 'else' branch cannot permanently change the typestate of an outer variable.");
					}
				}
			}
		}

		for (size_t i = 0; i < symTableBefore.size(); ++i)
		{
			for (const auto& [varName, symInfoBefore] : symTableBefore[i])
			{
				if (symTableAfterThen[i].contains(varName))
				{
					const auto& symInfoThen = symTableAfterThen[i].at(varName);
					if (symInfoBefore.isMoved != symInfoThen.isMoved)
					{
						throw SemanticError(node.location,
							"Variable '" + varName + "' was moved inside an 'if' without an 'else'. This leads to an ambiguous state at runtime.");
					}
				}
			}
		}

		m_symbolTable = symTableBefore;
		m_tsopVars = tsopBefore;
	}
}

void SemanticAnalyzer::Visit(WhileNode& node)
{
	AcceptNode(node.condition);

	EnterScope();
	m_loopDepth++;
	AcceptNode(node.body);
	m_loopDepth--;
	ExitScope();
}

void SemanticAnalyzer::Visit(ForNode& node)
{
	EnterScope();

	AcceptNode(node.init);
	AcceptNode(node.condition);
	AcceptNode(node.increment);
	m_loopDepth++;
	AcceptNode(node.body);
	m_loopDepth--;

	ExitScope();
}

void SemanticAnalyzer::Visit(CallNode& node)
{
	for (const auto& arg : node.args)
	{
		AcceptNode(arg);
	}
	if (node.funcName == "append" || node.funcName == "concat")
	{
		if (node.args.size() != 2)
		{
			throw SemanticError(node.location, "'" + node.funcName + "' expects exactly 2 arguments.");
		}

		const auto& baseArg = node.args[0];
		const auto& extArg = node.args[1];

		if (baseArg->arrayDepth == 0)
		{
			throw SemanticError(node.location, "First argument to '" + node.funcName + "' must be an array.");
		}

		if (node.funcName == "append")
		{
			if (extArg->evaluatedType != DataType::UNKNOWN)
			{
				if (baseArg->evaluatedType != extArg->evaluatedType
					|| baseArg->arrayDepth - 1 != extArg->arrayDepth
					|| baseArg->structName != extArg->structName)
				{
					throw SemanticError(node.location, "Type mismatch in 'append'. Second argument must be a single element of the array's type.");
				}
			}
		}
		else if (node.funcName == "concat")
		{
			if (extArg->evaluatedType != DataType::UNKNOWN)
			{
				if (baseArg->evaluatedType != extArg->evaluatedType
					|| baseArg->arrayDepth != extArg->arrayDepth
					|| baseArg->structName != extArg->structName)
				{
					throw SemanticError(node.location, "Type mismatch in 'concat'. Both arguments must be arrays of the exact same type and depth.");
				}
			}
		}

		node.evaluatedType = baseArg->evaluatedType;
		node.arrayDepth = baseArg->arrayDepth;
		node.structName = baseArg->structName;

		return;
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
		throw SemanticError(node.location, "Call to undefined function '" + node.funcName + "'.");
	}
}

void SemanticAnalyzer::Visit(FunctionDeclNode& node)
{
	m_functions[node.name] = { node.returnType, node.arrayDepth, node.structName, false };

	for (const auto& capNode : node.captures)
	{
		if (capNode)
		{
			bool found = false;
			SymbolInfo outerVar{};
			for (const auto& scope : std::ranges::reverse_view(m_symbolTable))
			{
				if (scope.contains(capNode->varName))
				{
					outerVar = scope.at(capNode->varName);
					found = true;
					break;
				}
			}
			if (!found)
			{
				throw SemanticError(node.location, "Cannot capture undefined variable '" + capNode->varName + "'");
			}
			if (outerVar.type != capNode->varType || outerVar.arrayDepth != capNode->arrayDepth || outerVar.structName != capNode->structName)
			{
				throw SemanticError(node.location, "Type mismatch for captured variable '" + capNode->varName + "'");
			}
		}
	}

	const auto previousSignature = m_currentFunctionSignature;
	m_currentFunctionSignature = { node.returnType, node.arrayDepth, node.structName, false };

	EnterScope();

	for (const auto& capNode : node.captures)
	{
		if (capNode)
		{
			m_symbolTable.back()[capNode->varName] = {
				capNode->varType,
				capNode->arrayDepth,
				capNode->structName,
				capNode->isConst,
			};
		}
	}

	for (const auto& paramNode : node.parameters)
	{
		if (paramNode)
		{
			m_symbolTable.back()[paramNode->varName] = {
				paramNode->varType,
				paramNode->arrayDepth,
				paramNode->structName,
				paramNode->isConst,
			};
		}
	}

	AcceptNode(node.body);
	ExitScope();
	m_currentFunctionSignature = previousSignature;
}

void SemanticAnalyzer::Visit(StructDeclNode& node)
{
	StructSignature signature;
	EnterScope();
	for (const auto& field : node.fields)
	{
		if (const auto varDecl = std::dynamic_pointer_cast<VarDeclNode>(field))
		{
			signature.fieldTypes[varDecl->varName] = {
				varDecl->varType,
				varDecl->arrayDepth,
				varDecl->structName,
				varDecl->isConst
			};
			signature.orderedFields.push_back(varDecl->varName);
			signature.fieldCount++;
		}
		AcceptNode(field);
	}
	ExitScope();
	m_structSignatures[node.name] = std::move(signature);
}

void SemanticAnalyzer::Visit(StructInitNode& node)
{
	if (!m_structSignatures.contains(node.structName))
	{
		throw SemanticError(node.location, "Unknown struct '" + node.structName + "'");
	}

	const auto& signature = m_structSignatures[node.structName];

	if (node.args.size() > signature.fieldCount)
	{
		throw SemanticError(
			node.location,
			"Too many arguments for struct '" + node.structName
				+ "'. Expected max " + std::to_string(signature.fieldCount) + ".");
	}

	for (const auto& arg : node.args)
	{
		AcceptNode(arg);
	}

	node.evaluatedType = DataType::INSTANCE;
	node.arrayDepth = 0;
}

void SemanticAnalyzer::Visit(MemberAccessNode& node)
{
	if (const auto varAccess = std::dynamic_pointer_cast<VarAccessNode>(node.object))
	{
		if (m_enums.contains(varAccess->varName))
		{
			varAccess->Accept(*this);
			const auto& enumName = varAccess->varName;
			const auto& [backingType, cases] = m_enums.at(enumName);

			if (!cases.contains(node.fieldName))
			{
				throw SemanticError(node.location, "Enum '" + enumName + "' does not have a case '" + node.fieldName + "'");
			}

			node.isEnumCase = true;
			node.enumLiteralValue = cases.at(node.fieldName);
			node.enumBackingType = backingType;

			node.evaluatedType = DataType::INSTANCE;
			node.structName = enumName;
			node.arrayDepth = 0;
			return;
		}
	}

	if (node.object)
	{
		node.object->Accept(*this);
		if (node.object->evaluatedType == DataType::INSTANCE && m_enums.contains(node.object->structName))
		{
			if (node.fieldName == "value")
			{
				const auto& [backingType, cases] = m_enums.at(node.object->structName);
				node.evaluatedType = backingType;
				node.arrayDepth = 0;

				if (const auto inner = std::dynamic_pointer_cast<MemberAccessNode>(node.object))
				{
					node.isEnumCase = true;
					node.enumLiteralValue = inner->enumLiteralValue;
					node.enumBackingType = inner->enumBackingType;
				}
				return;
			}
			throw SemanticError(node.location, "Enums only support the '.value' property.");
		}
	}

	if (node.object && node.object->evaluatedType == DataType::INSTANCE)
	{
		std::string structName = node.object->structName;

		if (m_nodeTypes.contains(node.object.get()))
		{
			const auto& [schema, state] = m_nodeTypes[node.object.get()];
			structName = schema + "_" + state;
		}

		if (m_structSignatures.contains(structName))
		{
			const auto& sig = m_structSignatures.at(structName);
			if (sig.fieldTypes.contains(node.fieldName))
			{
				const auto& fieldInfo = sig.fieldTypes.at(node.fieldName);

				node.evaluatedType = fieldInfo.type;
				node.arrayDepth = fieldInfo.arrayDepth;
				node.structName = fieldInfo.structName;
				return;
			}
			throw SemanticError(node.location, "Field '" + node.fieldName + "' does not exist in state/struct '" + structName + "'");
		}
	}

	throw SemanticError(node.location, "Cannot access field '" + node.fieldName + "'. Object is not a valid struct or state.");
}

void SemanticAnalyzer::Visit(MemberAssignmentNode& node)
{
	AcceptNode(node.object);
	AcceptNode(node.expression);
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
					throw SemanticError(node.location, "All elements in array literal must have the same exact type.");
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
	AcceptNode(node.target);
	AcceptNode(node.index);

	if (node.target
		&& node.target->arrayDepth == 0
		&& node.target->evaluatedType != DataType::UNKNOWN)
	{
		throw SemanticError(node.location, "Cannot index a non-array target.");
	}
	if (node.index && node.index->evaluatedType != DataType::UNKNOWN && node.index->evaluatedType != DataType::INT)
	{
		throw SemanticError(node.location, "Array index must be an integer.");
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
	AcceptNode(node.target);
	AcceptNode(node.index);
	AcceptNode(node.value);

	if (node.target && node.target->arrayDepth == 0 && node.target->evaluatedType != DataType::UNKNOWN)
	{
		throw SemanticError(node.location, "Cannot index a non-array target.");
	}
	if (node.index && node.index->evaluatedType != DataType::UNKNOWN && node.index->evaluatedType != DataType::INT)
	{
		throw SemanticError(node.location, "Array index must be an integer.");
	}

	if (node.target && node.value
		&& node.target->evaluatedType != DataType::UNKNOWN
		&& node.value->evaluatedType != DataType::UNKNOWN)
	{
		if (node.target->evaluatedType != node.value->evaluatedType
			|| node.target->structName != node.value->structName
			|| (node.target->arrayDepth - 1) != node.value->arrayDepth)
		{
			throw SemanticError(node.location, "Type mismatch in array assignment.");
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
			const bool isFloatPromotion = (m_currentFunctionSignature.type == DataType::FLOAT && node.expr->evaluatedType == DataType::INT);

			if (!isFloatPromotion
				&& (node.expr->evaluatedType != m_currentFunctionSignature.type
					|| node.expr->arrayDepth != m_currentFunctionSignature.arrayDepth
					|| node.expr->structName != m_currentFunctionSignature.structName))
			{
				auto errorMsg = "Return type mismatch. Expected type " + DATA_TYPE_NAMES.at(m_currentFunctionSignature.type);
				if (m_currentFunctionSignature.arrayDepth > 0)
				{
					for (int i = 0; i < m_currentFunctionSignature.arrayDepth; ++i)
					{
						errorMsg += "[]";
					}
				}
				throw SemanticError(node.location, errorMsg);
			}
		}
		return;
	}
	if (m_currentFunctionSignature.type != DataType::UNKNOWN
		&& m_currentFunctionSignature.type != DataType::VOID)
	{
		throw SemanticError(node.location, "Empty return statement in a non-void function.");
	}
}

void SemanticAnalyzer::Visit(ImportNode& node)
{
	if (node.filePath.ends_with(".stasm"))
	{
		ImportModuleMetadata(node.filePath);
	}
}

void SemanticAnalyzer::Visit(SchemaFuncNode& node)
{
	EnterScope();
	for (const auto& param : node.parameters)
	{
		AcceptNode(param);
	}
	ExitScope();
}

void SemanticAnalyzer::Visit(SchemaStateNode& node)
{
	EnterScope();
	for (const auto& field : node.fields)
	{
		AcceptNode(field);
	}
	ExitScope();
}

void SemanticAnalyzer::Visit(SchemaDeclNode& node)
{
	if (m_schemas.contains(node.name))
	{
		throw SemanticError(node.location, "Schema '" + node.name + "' is already defined.");
	}

	SchemaSignature schemaSig;
	std::unordered_set<std::string> stateNames;
	for (const auto& state : node.states)
	{
		if (state)
		{
			stateNames.insert(state->name);
		}
	}
	for (const auto& state : node.states)
	{
		if (state)
		{
			StateSignature stateSig;
			stateSig.fieldCount = state->fields.size();

			for (const auto& method : state->methods)
			{
				if (method)
				{
					if (!stateNames.contains(method->returnState))
					{
						throw SemanticError(method->location,
							"Transition to undefined state '"
								+ method->returnState + "' in schema '"
								+ node.name + '\'');
					}

					MethodSignature methSig;
					methSig.returnState = method->returnState;
					for (const auto& param : method->parameters)
					{
						methSig.paramTypes.push_back(param->varType);
						methSig.paramStructNames.push_back(param->structName);
					}
					stateSig.transitions[method->name] = methSig;
				}
			}
			schemaSig.states[state->name] = stateSig;

			StructSignature structSig;
			structSig.fieldCount = state->fields.size();
			for (const auto& field : state->fields)
			{
				if (const auto varDecl = std::dynamic_pointer_cast<VarDeclNode>(field))
				{
					ValidateTypeExists(varDecl->varType, varDecl->structName, varDecl->location);

					structSig.fieldTypes[varDecl->varName] = {
						varDecl->varType,
						varDecl->arrayDepth,
						varDecl->structName,
						varDecl->isConst
					};
					structSig.orderedFields.push_back(varDecl->varName);
				}
			}
			m_structSignatures[node.name + "_" + state->name] = std::move(structSig);

			state->Accept(*this);
		}
	}

	m_schemas[node.name] = schemaSig;
}

void SemanticAnalyzer::Visit(ImplDeclNode& node)
{
	if (!m_schemas.contains(node.schemaName))
	{
		throw SemanticError(
			node.location, "Schema '" + node.schemaName + "' is not declared.");
	}

	auto& [states] = m_schemas.at(node.schemaName);
	if (!states.contains(node.stateName))
	{
		throw SemanticError(node.location, "State '" + node.stateName + "' does not exist in schema '" + node.schemaName + "'.");
	}

	EnterSchemaContext(node);

	std::unordered_set<std::string> implementedMethods;
	for (const auto& method : node.methods)
	{
		if (method)
		{
			implementedMethods.insert(method->name);
			method->Accept(*this);
		}
	}

	const auto& requiredTransitions = states.at(node.stateName).transitions;
	for (const auto& reqName : requiredTransitions | std::views::keys)
	{
		if (!implementedMethods.contains(reqName))
		{
			throw SemanticError(node.location, "State '" + node.stateName + "' is missing implementation for function '" + reqName + "' declared in schema.");
		}
	}

	DropSchemaContext();
}

void SemanticAnalyzer::Visit(MethodDeclNode& node)
{
	const auto& transitions = m_schemas
								  .at(m_currentImplSchema)
								  .states
								  .at(m_currentImplState)
								  .transitions;

	if (!transitions.contains(node.name))
	{
		throw SemanticError(node.location, "Method '" + node.name + "' is not declared in schema state '" + m_currentImplState + "'.");
	}
	if (transitions.at(node.name).returnState != node.targetState->stateName)
	{
		throw SemanticError(node.location, "Method '" + node.name + "' must return state '" + transitions.at(node.name).returnState + "', but returns '" + node.targetState->stateName + "'.");
	}

	EnterScope();
	m_symbolTable.back()["this"] = {
		DataType::INSTANCE,
		0,
		m_currentImplSchema + "_" + m_currentImplState,
		true
	};
	m_tsopVars.back()["this"] = { m_currentImplSchema, m_currentImplState };
	for (const auto& param : node.parameters)
	{
		if (param)
		{
			ValidateTypeExists(param->varType, param->structName, param->location);
			param->Accept(*this);
		}
	}

	AcceptNode(node.body);
	ExitScope();
}

void SemanticAnalyzer::Visit(TsopInitNode& node)
{
	if (!m_schemas.contains(node.schemaName)
		|| !m_schemas.at(node.schemaName).states.contains(node.stateName))
	{
		throw SemanticError(node.location, "Cannot initialize unknown TSOP state '" + node.schemaName + "<" + node.stateName + ">'.");
	}

	const auto& [transitions, fieldCount] = m_schemas.at(node.schemaName).states.at(node.stateName);
	if (node.args.size() != fieldCount)
	{
		throw SemanticError(node.location,
			"Argument count mismatch. State '"
				+ node.stateName + "' expects " + std::to_string(fieldCount) + " arguments, but got "
				+ std::to_string(node.args.size()));
	}
	for (const auto& arg : node.args)
	{
		AcceptNode(arg);
	}
	m_nodeTypes[&node] = { node.schemaName, node.stateName };
	node.evaluatedType = DataType::INSTANCE;
	node.structName = node.schemaName + "_" + node.stateName;
	node.arrayDepth = 0;
}

void SemanticAnalyzer::Visit(ImplStateNode& node)
{
	if (!m_schemas.contains(node.schemaName)
		|| !m_schemas
			.at(node.schemaName)
			.states
			.contains(node.stateName))
	{
		throw SemanticError(node.location, "Unknown TSOP state '" + node.schemaName + "<" + node.stateName + ">'.");
	}
}

void SemanticAnalyzer::Visit(MethodCallNode& node)
{
	AcceptNode(node.object);
	if (node.object->evaluatedType != DataType::INSTANCE || node.object->structName.empty())
	{
		throw SemanticError(node.location, "Cannot call method '." + node.methodName + "' on a non-TSOP object or undefined variable.");
	}

	const auto objStructName = node.object->structName;
	const auto delimPos = objStructName.find('_');
	if (delimPos == std::string::npos)
	{
		throw SemanticError(node.location, "Object '" + objStructName + "' is not a valid TSOP state.");
	}

	const auto schema = objStructName.substr(0, delimPos);
	const auto state = objStructName.substr(delimPos + 1);

	const auto& [transitions, fieldCount] = m_schemas.at(schema).states.at(state);
	if (!transitions.contains(node.methodName))
	{
		throw SemanticError(node.location, "Method '" + node.methodName + "' does not exist in state '" + schema + "_" + state + "'.");
	}

	const auto& [returnState, paramTypes, paramStructNames] = transitions.at(node.methodName);
	if (node.args.size() != paramTypes.size())
	{
		throw SemanticError(node.location, "Argument count mismatch in method '" + node.methodName + "'. Expected " + std::to_string(paramTypes.size()));
	}

	for (size_t i = 0; i < node.args.size(); ++i)
	{
		if (node.args[i])
		{
			node.args[i]->Accept(*this);

			if (node.args[i]->evaluatedType != paramTypes[i] || (node.args[i]->evaluatedType == DataType::INSTANCE && node.args[i]->structName != paramStructNames[i]))
			{
				throw SemanticError(node.args[i]->location, "Type mismatch for argument " + std::to_string(i + 1) + " in method '" + node.methodName + "'");
			}
		}
	}

	node.mangledFuncName = schema + "_" + state + "_" + node.methodName;

	node.evaluatedType = DataType::INSTANCE;
	node.structName = schema + "_" + returnState;
	node.arrayDepth = 0;
	m_nodeTypes[&node] = { schema, returnState };
}

void SemanticAnalyzer::Visit(EnumCaseNode& node)
{
	AcceptNode(node.value);
}

void SemanticAnalyzer::Visit(EnumDeclNode& node)
{
	if (m_enums.contains(node.name)
		|| m_structSignatures.contains(node.name)
		|| m_schemas.contains(node.name))
	{
		throw SemanticError(node.location, "Type '" + node.name + "' is already defined.");
	}

	EnumSignature sig;
	sig.backingType = node.backingType;

	for (const auto& caseNode : node.cases)
	{
		if (caseNode)
		{
			caseNode->Accept(*this);

			if (const auto literal = std::dynamic_pointer_cast<LiteralNode>(caseNode->value))
			{
				if (literal->evaluatedType != sig.backingType)
				{
					throw SemanticError(caseNode->location, "Type mismatch in enum case '" + caseNode->name + "'. Expected " + DATA_TYPE_NAMES.at(sig.backingType));
				}

				auto valStr = literal->token.value;
				if (sig.backingType == DataType::STRING)
				{
					valStr = valStr.substr(1, valStr.size() - 2);
				}
				sig.cases[caseNode->name] = valStr;
			}
			else
			{
				throw SemanticError(caseNode->location, "Enum case value must be a constant literal.");
			}
		}
	}
	m_enums[node.name] = std::move(sig);
}

void SemanticAnalyzer::Visit(MatchNode& node)
{
	if (node.expr)
	{
		node.expr->Accept(*this);
	}

	const auto targetType = node.expr ? node.expr->evaluatedType : DataType::UNKNOWN;
	const auto targetStruct = node.expr
		? node.expr->structName
		: "";

	auto returnType = DataType::UNKNOWN;
	std::string returnStruct;
	int returnDepth = 0;
	bool isFirstCase = true;

	for (const auto& matchCase : node.cases)
	{
		if (matchCase)
		{
			if (matchCase->pattern)
			{
				matchCase->pattern->Accept(*this);

				if (matchCase->pattern->evaluatedType != targetType || matchCase->pattern->structName != targetStruct)
				{
					throw SemanticError(matchCase->pattern->location, "Pattern type does not match the target expression type.");
				}
			}
			if (matchCase->body)
			{
				matchCase->body->Accept(*this);

				if (isFirstCase)
				{
					returnType = matchCase->body->evaluatedType;
					returnStruct = matchCase->body->structName;
					returnDepth = matchCase->body->arrayDepth;
					isFirstCase = false;
				}
				else
				{
					if (matchCase->body->arrayDepth != returnDepth)
					{
						throw SemanticError(matchCase->body->location, "Array depth mismatch between match cases.");
					}
					if (matchCase->body->evaluatedType == returnType)
					{
						if (returnType == DataType::INSTANCE && !IsTsopMatch(returnStruct, matchCase->body->structName))
						{
							throw SemanticError(matchCase->body->location, "Struct type mismatch between match cases.");
						}
					}
					else if ((returnType == DataType::INT && matchCase->body->evaluatedType == DataType::FLOAT)
						|| (returnType == DataType::FLOAT && matchCase->body->evaluatedType == DataType::INT))
					{
						returnType = DataType::FLOAT;
						returnStruct = "";
					}
					else
					{
						throw SemanticError(matchCase->body->location, "Incompatible return types between match cases.");
					}
				}
			}
		}
	}

	node.evaluatedType = returnType;
	node.structName = returnStruct;
	node.arrayDepth = returnDepth;
}

void SemanticAnalyzer::Visit(MatchCaseNode& node)
{
}

void SemanticAnalyzer::Visit(BreakNode& node)
{
	if (m_loopDepth == 0)
	{
		throw std::runtime_error("'break' outside of loop");
	}
}

void SemanticAnalyzer::Visit(ContinueNode& node)
{
	if (m_loopDepth == 0)
	{
		throw std::runtime_error("'continue' outside of loop");
	}
}

void SemanticAnalyzer::EnterScope()
{
	m_symbolTable.emplace_back();
	m_tsopVars.emplace_back();
}

void SemanticAnalyzer::ExitScope()
{
	if (!m_symbolTable.empty())
	{
		m_symbolTable.pop_back();
	}
	if (!m_tsopVars.empty())
	{
		m_tsopVars.pop_back();
	}
}

void SemanticAnalyzer::EnterSchemaContext(const ImplNode& node)
{
	m_currentImplSchema = node.schemaName;
	m_currentImplState = node.stateName;
}

void SemanticAnalyzer::DropSchemaContext()
{
	m_currentImplSchema = "";
	m_currentImplState = "";
}

void SemanticAnalyzer::ValidateTypeExists(const DataType type, const std::string& structName, const SourceLocation& loc) const
{
	if (type != DataType::INSTANCE)
	{
		return;
	}
	if (!m_structSignatures.contains(structName)
		&& !m_schemas.contains(structName)
		&& !m_enums.contains(structName))
	{
		throw SemanticError(loc, "Unknown type '" + structName + "'.");
	}
}
void SemanticAnalyzer::AcceptNode(const std::shared_ptr<AstNode>& node)
{
	if (node)
	{
		node->Accept(*this);
	}
}

void SemanticAnalyzer::RegisterNativeLib()
{
	// base
	m_functions["time"] = { DataType::INT, 0, "", false };
	m_functions["len"] = { DataType::INT, 0, "", false };
	m_functions["to_int"] = { DataType::INT, 0, "", false };
	m_functions["to_str"] = { DataType::STRING, 0, "", false };
	m_functions["print"] = { DataType::VOID, 0, "", false };
	m_functions["read"] = { DataType::STRING, 0, "", false };
	m_functions["panic"] = { DataType::VOID, 0, "", false };
	m_functions["append"] = { DataType::UNKNOWN, 1, "", false };
	m_functions["concat"] = { DataType::UNKNOWN, 1, "", false };

	// file
	m_functions["file_exists"] = { DataType::BOOL, 0, "", false };
	m_functions["file_read"] = { DataType::STRING, 0, "", false };
	m_functions["file_write"] = { DataType::BOOL, 0, "", false };

	// sql
	m_functions["db_connect"] = { DataType::INT, 0, "", false };
	m_functions["db_execute"] = { DataType::STRING, 2, "", false };
	m_functions["db_disconnect"] = { DataType::VOID, 0, "", false };
	m_functions["sql_escape"] = { DataType::STRING, 0, "", false };

	// http
	m_functions["http_init"] = { DataType::INT, 0, "", false };
	m_functions["http_setopt"] = { DataType::VOID, 0, "", false };
	m_functions["http_send"] = { DataType::VOID, 0, "", false };
	m_functions["http_status"] = { DataType::INT, 0, "", false };
	m_functions["http_response"] = { DataType::STRING, 0, "", false };
	m_functions["http_cleanup"] = { DataType::VOID, 0, "", false };

	m_functions["http_send_async"] = { DataType::VOID, 0, "", false };
	m_functions["http_is_ready"] = { DataType::BOOL, 0, "", false };
	m_functions["http_await"] = { DataType::VOID, 0, "", false };

	// http server
	m_functions["http_server_start"] = { DataType::INT, 0, "", false };
	m_functions["http_server_stop"] = { DataType::VOID, 0, "", false };
	m_functions["http_server_accept"] = { DataType::INT, 0, "", false };

	// http request handling
	m_functions["http_request_method"] = { DataType::STRING, 0, "", false };
	m_functions["http_request_path"] = { DataType::STRING, 0, "", false };
	m_functions["http_request_header"] = { DataType::STRING, 0, "", false };
	m_functions["http_request_body"] = { DataType::STRING, 0, "", false };
	m_functions["http_request_respond"] = { DataType::INT, 0, "", false };

	// threads
	m_functions["thread_spawn"] = { DataType::INT, 0, "", false };
	m_functions["thread_join"] = { DataType::UNKNOWN, 0, "", false };

	// json
	m_functions["native_json_to_map"] = { DataType::INSTANCE, 0, "StringMap", false };
	m_functions["native_map_to_json"] = { DataType::STRING, 0, "", false };
	m_functions["json_escape"] = { DataType::STRING, 0, "", false };

	m_functions["ai_parse_response"] = { DataType::STRING, 0, "", false };
}

void SemanticAnalyzer::ExportModuleMetadata(const std::string& filepath) const
{
	nlohmann::json jsonData;

	jsonData["functions"] = m_functions;
	jsonData["structs"] = m_structSignatures;
	jsonData["schemas"] = m_schemas;
	jsonData["enums"] = m_enums;

	std::ofstream out(filepath);
	if (!out.is_open())
	{
		throw std::runtime_error("Could not open file for writing module: " + filepath);
	}
	out << jsonData.dump(4);
}

void SemanticAnalyzer::ImportModuleMetadata(const std::string& filepath)
{
	std::ifstream in(filepath);
	if (!in.is_open())
	{
		throw std::runtime_error("Could not open module file: " + filepath);
	}

	nlohmann::json jsonData;
	in >> jsonData;

	auto funcs = jsonData["functions"]
					 .get<std::unordered_map<std::string, SymbolInfo>>();
	m_functions.insert(funcs.begin(), funcs.end());

	auto structs = jsonData["structs"]
					   .get<std::unordered_map<std::string, StructSignature>>();
	m_structSignatures.insert(structs.begin(), structs.end());

	auto schemas = jsonData["schemas"]
					   .get<std::unordered_map<std::string, SchemaSignature>>();
	m_schemas.insert(schemas.begin(), schemas.end());

	auto enums = jsonData["enums"]
					 .get<std::unordered_map<std::string, EnumSignature>>();
	m_enums.insert(enums.begin(), enums.end());
}

bool IsTsopMatch(const std::string& left, const std::string& right)
{
	if (left == right)
	{
		return true;
	}

	auto getBaseSchema = [](const std::string& name) {
		const auto pos = name.find('_');
		return pos == std::string::npos ? name : name.substr(0, pos);
	};

	const auto leftBase = getBaseSchema(left);
	const auto rightBase = getBaseSchema(right);

	return leftBase == rightBase && !leftBase.empty();
}