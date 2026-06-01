#include "SchemaVisualizer.h"

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
	std::cout << "```mermaid\n";
	std::cout << "stateDiagram-v2\n";
	std::cout << "    direction LR\n\n";

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

			std::cout << "    " << state->name
					  << " --> " << method->returnState
					  << " : " << method->name << "\n";
		}
	}

	std::cout << "```\n";
}
