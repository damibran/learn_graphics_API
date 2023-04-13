#pragma once

namespace dmbrn
{
	struct BoneComponent
	{
		BoneComponent(size_t ind): bone_ind(ind)
		{
		}

		bool need_gpu_state_update = true;
		size_t bone_ind;
	};
}
