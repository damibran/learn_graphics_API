#pragma once

#include "Main/Enttity.h"

namespace dmbrn
{
	struct SkeletonComponent
	{
		SkeletonComponent(std::vector<Enttity>&& bone_entts):
			in_GPU_mtxs_offset(Renderer::per_skeleton_data_.registerObject()),
			bone_enttities(bone_entts)
		{
		}

		uint32_t in_GPU_mtxs_offset = 0;
		std::vector<Enttity> bone_enttities;
	};
}
