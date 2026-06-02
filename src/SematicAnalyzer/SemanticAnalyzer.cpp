#include "SemanticAnalyzer.h"
#include "../ErrorHandler/ErrorHandler.h"

#include <ranges>
#include <unordered_set>

void AcceptNode(AstNode& node);

void SemanticAnalyzer::Analyze(const std::shared_ptr<AstNode>& root)
{
	m_symbolTable.clear();
	EnterScope();

	AcceptNode(root);
	ExitScope();
}

void SemanticAnalyzer::Visit(TerminalNode& node)
{
}

void SemanticAnalyzer::Visit(CommonNode& node)
{
	for (const auto& child : node.children)
	{
		AcceptNode(child);
	}
}

void SemanticAnalyzer::Visit(BinaryOpNode& node)
{
	AcceptNode(node.left);
	AcceptNode(node.right);

	if (node.left && node.right)
	{
		if (node.left->evaluatedType != node.right->evaluatedType)
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
			throw SemanticError(node.location, "Unary operations on arrays are not allowed.");
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
			throw SemanticError(node.location, "Type mismatch for " + node.varName);
		}
		if (node.expr->arrayDepth != node.arrayDepth)
		{
			throw SemanticError(node.location, "Array depth mismatch for " + node.varName);
		}
		if (node.expr->evaluatedType == DataType::INSTANCE
			&& node.expr->structName != node.structName)
		{
			throw SemanticError(node.location, "Struct type mismatch for " + node.varName);
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
	auto itSym = m_symbolTable.rbegin();
	auto itTsop = m_tsopVars.rbegin();

	for (; itSym != m_symbolTable.rend(); ++itSym, ++itTsop)
	{
		if (itSym->contains(node.varName))
		{
			const auto& varInfo = itSym->at(node.varName);
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
	throw SemanticError(node.location, "Undefined variable '" + node.varName + "'");
}

void SemanticAnalyzer::Visit(AssignmentNode& node)
{
	AcceptNode(node.expr);

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
		throw SemanticError(node.location, "Assignment to undefined variable '" + node.varName + "'");
	}
	if (varInfo.isConst)
	{
		throw SemanticError(node.location, "Cannot assign to constant variable '" + node.varName + "'");
	}
	if (!node.expr)
	{
		return;
	}
	if (node.expr->evaluatedType != DataType::UNKNOWN && node.expr->evaluatedType != varInfo.type)
	{
		throw SemanticError(node.location, "Type mismatch in assignment to '" + node.varName + "'");
	}
	if (node.expr->arrayDepth != varInfo.arrayDepth)
	{
		throw SemanticError(node.location, "Array depth mismatch in assignment to '" + node.varName + "'");
	}
	if (node.expr->evaluatedType == DataType::INSTANCE && node.expr->structName != varInfo.structName)
	{
		throw SemanticError(node.location, "Struct type mismatch in assignment to '" + node.varName + "'");
	}
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

	EnterScope();
	AcceptNode(node.thenBranch);
	ExitScope();

	EnterScope();
	AcceptNode(node.elseBranch);
	ExitScope();
}

void SemanticAnalyzer::Visit(WhileNode& node)
{
	AcceptNode(node.condition);

	EnterScope();
	AcceptNode(node.body);
	ExitScope();
}

void SemanticAnalyzer::Visit(ForNode& node)
{
	EnterScope();

	AcceptNode(node.init);
	AcceptNode(node.condition);
	AcceptNode(node.increment);
	AcceptNode(node.body);

	ExitScope();
}

void SemanticAnalyzer::Visit(CallNode& node)
{
	for (const auto& arg : node.args)
	{
		AcceptNode(arg);
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
			signature.fieldCount++;
		}
		AcceptNode(field);
	}
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
	AcceptNode(node.object);

	if (node.object && node.object->evaluatedType == DataType::INSTANCE)
	{
		std::string structName = node.object->structName;

		if (m_nodeTypes.contains(node.object.get()))
		{
			const auto& tsopType = m_nodeTypes[node.object.get()];
			structName = tsopType.schema + "_" + tsopType.state;
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
			throw SemanticError(node.location, "Field '" + node.fieldName + "' does not exist in state '" + structName + "'");
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
			if (node.expr->evaluatedType != m_currentFunctionSignature.type
				|| node.expr->arrayDepth != m_currentFunctionSignature.arrayDepth
				|| node.expr->structName != m_currentFunctionSignature.structName)
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
					stateSig.transitions[method->name] = method->returnState;
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
	if (transitions.at(node.name) != node.targetState->stateName)
	{
		throw SemanticError(node.location, "Semantic error: Method '" + node.name + "' must return state '" + transitions.at(node.name) + "', but returns '" + node.targetState->stateName + "'.");
	}

	EnterScope();
	m_symbolTable.back()["this"] = {
		DataType::INSTANCE,
		0,
		m_currentImplSchema + "_" + m_currentImplState,
		true,
	};
	m_tsopVars.back()["this"] = { m_currentImplSchema, m_currentImplState };
	for (const auto& param : node.parameters)
	{
		if (param)
		{
			if (const auto varDecl = std::dynamic_pointer_cast<VarDeclNode>(param))
			{
				ValidateTypeExists(varDecl->varType, varDecl->structName, varDecl->location);
			}
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

	if (!m_nodeTypes.contains(node.object.get()))
	{
		throw SemanticError(node.location, "Semantic error: Cannot call method '." + node.methodName + "' on a non-TSOP object or undefined variable.");
	}

	const auto objType = m_nodeTypes[node.object.get()];

	const auto& [transitions, fieldCount] = m_schemas.at(objType.schema).states.at(objType.state);
	if (!transitions.contains(node.methodName))
	{
		throw SemanticError(node.location, "Semantic error: Method '" + node.methodName + "' does not exist in state '" + objType.schema + "<" + objType.state + ">'.");
	}

	const auto returnState = transitions.at(node.methodName);
	m_nodeTypes[&node] = { objType.schema, returnState };

	node.mangledFuncName = objType.schema + "_" + objType.state + "_" + node.methodName;
	for (const auto& arg : node.args)
	{
		AcceptNode(arg);
	}
	node.evaluatedType = DataType::INSTANCE;
	node.structName = objType.schema;
	node.arrayDepth = 0;
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

void SemanticAnalyzer::ValidateTypeExists(DataType type, const std::string& structName, const SourceLocation& loc) const
{
	if (type != DataType::INSTANCE)
	{
		return;
	}
	if (!m_structSignatures.contains(structName) && !m_schemas.contains(structName))
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
