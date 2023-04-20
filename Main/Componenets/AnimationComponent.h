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
		bool playing = false;
		unsigned playing_ind = 0;
		time_point start_time;

		duration getLocalTime(time_point g_time)
		{
			duration l_time;
			AnimationClip& playing_clip = animation_clips[playing_ind];

			double duration_s = playing_clip.duration_.count();
			
			if (loop)
				l_time = duration{std::fmod((g_time - start_time).count(), duration_s)};
			else
				l_time = duration{glm::clamp((g_time - start_time).count(), 0., duration_s)};

			return l_time;
		}
	};
}
