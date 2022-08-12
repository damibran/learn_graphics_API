#pragma once
#include <string>
#include <glm/glm.hpp>

namespace dmbrn
{
	struct TagComponent
	{
		std::string tag;
		TagComponent(const std::string& name) :
			tag(name)
		{}
	};

	struct TransformComponent
	{
		glm::vec3 postition;
		glm::vec3 rotation;
		glm::vec3 scale;
		TransformComponent(glm::vec3 pos = { 0,0,0 },
			glm::vec3 rot = { 0,0,0 },
			glm::vec3 scale = { 1,1,1 }):
			postition(pos),
			rotation(rot),
			scale(scale)
		{}
	};

	struct MeshRenderrer
	{
	
	};
}