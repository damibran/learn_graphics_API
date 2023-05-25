#pragma once

#include "ECS/Enttity.h"

namespace dmbrn
{
	/**
	 * \brief Component means that entity that have it is root of skeleton
	 */
	struct SkeletonComponent
	{
		/**
		 * \brief register skeleton in registerObject and moves bone_entts
		 * \param bone_entts movable entity id's
		 */
		SkeletonComponent(std::vector<Enttity>&& bone_entts):
			in_GPU_mtxs_offset(Renderer::per_skeleton_data_.registerObject()),
			bone_enttities(bone_entts)
		{
		}

		/**
		 * \brief is offset to object in PerSkeletonData buffer
		 */
		uint32_t in_GPU_mtxs_offset = 0;

		/**
		 * \brief array of entity id's which makeup this skeleton
		 */
		std::vector<Enttity> bone_enttities;
	};
}
