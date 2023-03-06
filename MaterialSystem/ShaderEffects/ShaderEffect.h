#pragma once
#include <glm/glm.hpp>

#include <queue>
#include <Wrappers/Mesh.h>
#include <Wrappers/Singletons/PerObjectDataBuffer.h>
#include <MaterialSystem/Materials/Material.h>

namespace dmbrn
{
	struct ShaderEffect
	{
		virtual ~ShaderEffect() = default;
		virtual void draw(int frame, const vk::raii::CommandBuffer& command_buffer,const PerObjectDataBuffer& per_object_data_buffer)=0;
		void addToRenderQueue(std::pair<const Mesh*, size_t> pair)
		{
			render_queue.push_back(pair);
		}
	protected:
		std::vector<std::pair<const Mesh*, size_t>> render_queue;
	};
}
