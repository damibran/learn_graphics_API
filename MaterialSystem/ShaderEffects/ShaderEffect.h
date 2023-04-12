#pragma once

#include <set>
#include <Wrappers/Mesh.h>
#include <Wrappers/SkeletalMesh.h>

namespace dmbrn
{
	struct ShaderEffect
	{
		virtual ~ShaderEffect() = default;
		virtual void draw(int frame, const vk::raii::CommandBuffer& command_buffer)=0;
		void addToRenderQueue(std::pair<const Mesh*, size_t> pair)
		{
			static_render_queue.insert(pair);
		}
		void addToRenderQueue(std::pair<const SkeletalMesh*, size_t> pair)
		{
			skeletal_render_queue.insert(pair);
		}
	protected:
		std::multiset<std::pair<const Mesh*, size_t>> static_render_queue;
		std::multiset<std::pair<const SkeletalMesh*, size_t>> skeletal_render_queue;
	};
}
