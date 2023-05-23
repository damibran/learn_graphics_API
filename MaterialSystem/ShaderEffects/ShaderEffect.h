#pragma once

#include <set>
#include <Wrappers/Mesh.h>
#include <Wrappers/SkeletalMesh.h>

namespace std
{
	inline bool operator<(const std::pair<const dmbrn::Mesh*, uint32_t>& lhs,
	                      const std::pair<const dmbrn::Mesh*, uint32_t>& rhs)
	{
		if (lhs.first->render_data_ != rhs.first->render_data_)
			return lhs.first->render_data_ < rhs.first->render_data_;
		else
		{
			if (lhs.first->material_ != rhs.first->material_)
				return lhs.first->material_ < rhs.first->material_;
			else
				if (lhs.second != rhs.second)
					return lhs.second < rhs.second;
				else
					assert(false);
		}
	}

	inline bool operator<(const std::pair<const dmbrn::SkeletalMesh*, uint32_t>& lhs,
	                      const std::pair<const dmbrn::SkeletalMesh*, uint32_t>& rhs)
	{
		if (lhs.first->render_data_ != rhs.first->render_data_)
			return lhs.first->render_data_ < rhs.first->render_data_;
		else
		{
			if (lhs.first->material_ != rhs.first->material_)
				return lhs.first->material_ < rhs.first->material_;
			else
			{
				if (lhs.second != rhs.second)
					return lhs.second < rhs.second;
				else
					assert(false);
			}
		}
	}
}

namespace dmbrn
{
	/**
	 * \brief describes common interface for further shader effects
	 */
	struct ShaderEffect
	{
		virtual ~ShaderEffect() = default;
		virtual void draw(int frame, const vk::raii::CommandBuffer& command_buffer) =0;

		void addToRenderQueue(std::pair<const Mesh*, uint32_t> pair)
		{
			static_render_queue.insert(pair);
		}

		void addToRenderQueue(std::pair<const SkeletalMesh*, uint32_t> pair)
		{
			skeletal_render_queue.insert(pair);
		}

	protected:
		std::set<std::pair<const Mesh*, uint32_t>> static_render_queue;
		std::set<std::pair<const SkeletalMesh*, uint32_t>> skeletal_render_queue;
	};
}
