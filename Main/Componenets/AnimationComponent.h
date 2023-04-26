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

		AnimationComponent(std::set<AnimationClip>&& clips):
			animation_clips(clips)
		{
		}

		void insert(std::set<AnimationClip>&& new_clips)
		{
			// potentialy big copy!
			for (AnimationClip new_clip : new_clips)
			{
				int i=1;
				while(!animation_clips.insert(new_clip).second)
				{
					new_clip.name=new_clip.name+std::to_string(i);
				}
			}
		}

		std::set<AnimationClip> animation_clips;
	};
}
