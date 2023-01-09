#pragma once
#include <glm/glm.hpp>

namespace dmbrn
{
	struct Material
	{
		virtual ~Material() = default;
		virtual void draw(const vk::raii::Buffer& vertex_buffer ,const vk::raii::Buffer& index_buffer ,uint32_t indices_count,int frame, const vk::raii::CommandBuffer& command_buffer, const glm::mat4& modelMat,
		                  const glm::mat4& view,
		                  const glm::mat4& proj) const =0;
	};
}
