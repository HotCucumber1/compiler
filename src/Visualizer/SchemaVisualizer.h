#pragma once
#include "BaseVisualizer.h"

class SchemaVisualizer : public BaseVisualizer
{
public:
	explicit SchemaVisualizer(std::ostream& out);

	void Generate(const std::shared_ptr<AstNode>& root) override;

	void Visit(SchemaDeclNode& node) override;

private:
	std::ostream& m_out;
};
