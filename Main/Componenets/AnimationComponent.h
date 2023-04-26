#pragma once
#include <chrono>
#include <cmath>
#include "Wrappers/Animation.h"

namespace dmbrn
{
	struct KeyValues
	{
		std::unique_ptr<glm::vec3> pos;
		std::unique_ptr<glm::quat> rot;
		std::unique_ptr<glm::vec3> scale;
	};

	struct AnimationComponent
	{
		const bool loop =true;

		AnimationComponent() = default;

		AnimationComponent(std::vector<AnimationClip>&& clips):
			animation_clips(clips)
		{
		}

		std::vector<AnimationClip> animation_clips;
	};
}
