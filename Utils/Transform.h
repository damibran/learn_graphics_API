#pragma once
#include <glm/glm.hpp>


namespace dmbrn
{
	struct Transform
	{
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale = glm::vec3(1, 1, 1);
	};
} // namespace dmbrn