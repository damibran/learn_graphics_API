#pragma once

namespace dmbrn
{
	struct BoneComponent
	{
		BoneComponent(glm::mat4 o_mat,size_t ind): offset_mat(o_mat),bone_ind(ind)
		{
		}

		bool need_gpu_state_update = true;
		glm::mat4 offset_mat;
		size_t bone_ind;
	};
}
