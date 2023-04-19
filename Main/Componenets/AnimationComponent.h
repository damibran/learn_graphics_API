#pragma once

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
		const bool loop =false;

		AnimationComponent() = default;

		AnimationComponent(std::vector<AnimationClip>&& clips):
			animation_clips(clips)
		{
		}

		std::vector<AnimationClip> animation_clips;
		bool playing = false;
		unsigned playing_ind = 0;
		double start_time = 0;

		double getLocalTime(double g_time,double delta_t)
		{
			double l_time;
			AnimationClip& playing_clip = animation_clips[playing_ind];

			double duration = playing_clip.duration/delta_t;

			if (loop)
				l_time = std::modf(g_time - start_time, &duration);
			else
				l_time = glm::clamp(g_time - start_time, 0., duration);

			return l_time;
		}
	};
}
