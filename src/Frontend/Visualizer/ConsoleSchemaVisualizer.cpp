#include "ConsoleSchemaVisualizer.h"

void ConsoleSchemaVisualizer::Generate(const std::shared_ptr<AstNode>& root)
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

void ConsoleSchemaVisualizer::Visit(SchemaDeclNode& node)
{
	std::cout << "\n=== SCHEMA: " << node.name << " ===\n\n";

	for (const auto& state : node.states)
	{
		if (!state)
		{
			continue;
		}

		std::cout << "[" << state->name << "]\n";

		const size_t methodCount = state->methods.size();
		for (size_t i = 0; i < methodCount; ++i)
		{
			const auto& method = state->methods[i];
			if (!method)
			{
				continue;
			}

			const bool isLast = (i == methodCount - 1);
			const std::string branch = isLast
				? " └── "
				: " ├── ";

			std::cout << branch << "func " << method->name << "(";

			for (size_t p = 0; p < method->parameters.size(); ++p)
			{
				std::cout << method->parameters[p]->varName;
				if (p < method->parameters.size() - 1)
				{
					std::cout << ", ";
				}
			}
			std::cout << ") ──> [" << method->returnState << "]\n";
		}
		std::cout << "\n";
	}
}
