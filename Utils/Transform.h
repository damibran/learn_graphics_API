#pragma once
#include <glm/glm.hpp>


namespace dmbrn
{
	struct Transform
	{
		glm::vec3 position = glm::vec3{0, 0, 0};
		glm::vec3 rotation = glm::vec3{0, 0, 0};
		glm::vec3 scale = glm::vec3(1, 1, 1);
	};

	struct SceneNode
	{
		std::string name = "";
		Transform transform{};
		Mesh* mesh = nullptr;
		std::vector<SceneNode> children;
	};
} // namespace dmbrn
