#include "SchemaVisualizer.h"

SchemaVisualizer::SchemaVisualizer(std::ostream& out)
	: m_out(out)
{
}

void SchemaVisualizer::Generate(const std::shared_ptr<AstNode>& root)
{
	if (auto program = std::dynamic_pointer_cast<CommonNode>(root))
	{
		for (const auto& node : program->children)
		{
			if (node->type == AstNodeType::SCHEMA_DECL)
			{
				node->Accept(*this);
			}
		}
	}
}

void SchemaVisualizer::Visit(SchemaDeclNode& node)
{
	m_out << "```mermaid\n";
	m_out << "---\ntitle: " << node.name << "\n---\n";
	m_out << "stateDiagram-v2\n";
	m_out << "    direction LR\n\n";

	for (const auto& state : node.states)
	{
		if (!state)
		{
			continue;
		}

		for (const auto& method : state->methods)
		{
			if (!method)
			{
				continue;
			}

			m_out << "    " << state->name
				  << " --> " << method->returnState
				  << " : " << method->name << std::endl;
		}
	}

	m_out << "```" << std::endl;
}
