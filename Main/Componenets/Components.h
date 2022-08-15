#pragma once
#include <string>
#include <glm/glm.hpp>

#include "Wrappers/Model.h"
#include "Materials/UnLitTextured/UnlitTextureMaterial.h"

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

	class MeshRendererComponent
	{
	public:
		MeshRendererComponent(const std::string& modelPath,const Singletons& singletons,const ViewportRenderPass& render_pass):
					material_(singletons,render_pass),
			model_(modelPath,singletons)
		{
		}

		void draw(int curentFrame, const LogicalDevice& device,const vk::raii::CommandBuffer& command_buffer )
		{
			model_.Draw(curentFrame,device,material_.descriptor_sets_,command_buffer,material_.descriptor_sets_);
		}

	private:
		UnlitTextureMaterial material_;
		Model model_;
	};
}