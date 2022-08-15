#pragma once
#include <string>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Wrappers/Model.h"
#include "Materials/UnLitTextured/UnlitTextureMaterial.h"

namespace dmbrn
{
	struct TagComponent
	{
		std::string tag;

		TagComponent(const std::string& name) :
			tag(name)
		{
		}
	};

	struct TransformComponent
	{
		glm::vec3 postition;
		glm::vec3 rotation;
		glm::vec3 scale;

		TransformComponent(glm::vec3 pos = {0, 0, 0},
		                   glm::vec3 rot = {0, 0, 0},
		                   glm::vec3 scale = {1, 1, 1}):
			postition(pos),
			rotation(rot),
			scale(scale)
		{
		}

		glm::mat4 getMatrix() const
		{
			glm::mat4 res = glm::scale(glm::mat4(1.0f), scale);
			res = rotate(res, rotation.x, {1, 0, 0});
			res = rotate(res, rotation.y, {0, 1, 0});
			res = rotate(res, rotation.z, {0, 0, 1});
			res = translate(res, postition);

			return res;
		}
	};

	class MeshRendererComponent
	{
	public:
		MeshRendererComponent(const std::string& modelPath, const Singletons& singletons,
		                      const ViewportRenderPass& render_pass):
			material_(singletons, render_pass),
			model_(modelPath, singletons)
		{
		}

		void draw(int curentFrame, const LogicalDevice& device, const vk::raii::CommandBuffer& command_buffer,
		          const TransformComponent& transform)
		{
			material_.updateUBO(curentFrame, transform.getMatrix());
			model_.draw(curentFrame, device, command_buffer, material_);
		}

	private:
		UnlitTextureMaterial material_;
		Model model_;
	};
}
