#pragma once
#include "BaseVisualizer.h"

class ConsoleSchemaVisualizer : public BaseVisualizer
{
public:
	void Generate(const std::shared_ptr<AstNode>& root) override;

	void Visit(SchemaDeclNode& node) override;
};
