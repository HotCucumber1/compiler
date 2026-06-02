#pragma once
#include "../Common/Node.h"

class AstFactory
{
public:
	static std::shared_ptr<AstNode> CreateNode(
		AstNodeType type,
		SymbolId id,
		const std::vector<std::shared_ptr<AstNode>>& children);
};
