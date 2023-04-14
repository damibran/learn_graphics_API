#pragma once

#include <set>
#include <Wrappers/Mesh.h>
#include <Wrappers/SkeletalMesh.h>

namespace dmbrn
{
	inline bool operator<(const std::pair<const SkeletalMesh*, SkeletalOffsets> &lhs, const std::pair<const SkeletalMesh*, SkeletalOffsets>&rhs )
	{
		return lhs.first<rhs.first;
	}

	struct ShaderEffect
	{
		virtual ~ShaderEffect() = default;
		virtual void draw(int frame, const vk::raii::CommandBuffer& command_buffer)=0;
		void addToRenderQueue(std::pair<const Mesh*, uint32_t> pair)
		{
			static_render_queue.insert(pair);
		}
		void addToRenderQueue(std::pair<const SkeletalMesh*, uint32_t> pair)
		{
			skeletal_render_queue.insert(pair);
		}
	protected:
		std::multiset<std::pair<const Mesh*, uint32_t>> static_render_queue;
		std::multiset<std::pair<const SkeletalMesh*, uint32_t>> skeletal_render_queue;
	};
}
