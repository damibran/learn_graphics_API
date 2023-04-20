#pragma once

#include<glm/glm.hpp>
#include <set>
#include <string>

#include "Main/Enttity.h"

namespace dmbrn
{
	struct AnimationChannels
	{
		Enttity enttity;
		std::map<duration, glm::vec3> positions;
		std::map<duration, glm::quat> rotations;
		std::map<duration, glm::vec3> scales;
	};

	struct AnimationClip
	{
		std::string name;
		duration duration_; // in seconds
		std::vector<AnimationChannels> channels;

		void updateTransforms(duration l_time, uint32_t frame)
		{
			for (auto& chnls : channels)
			{
				TransformComponent& ent_trans_c = chnls.enttity.getComponent<TransformComponent>();

				ent_trans_c.position = mixPositions(l_time, chnls);
				ent_trans_c.setQuat(slerpRotation(l_time, chnls));
				ent_trans_c.scale = mixScale(l_time, chnls);

				chnls.enttity.markTransformAsEdited(frame);
			}
		}

	private:
		double GetScaleFactor(double lastTimeStamp, double nextTimeStamp, double animationTime)
		{
			double midWayLength = animationTime - lastTimeStamp;
			double framesDiff = nextTimeStamp - lastTimeStamp;
			return midWayLength / framesDiff;
		}

		glm::vec3 mixPositions(duration l_time, const AnimationChannels& chnls)
		{
			auto [lb,ub] = chnls.positions.equal_range(l_time);

			if (lb != chnls.positions.begin())
				lb--;
			
			double factor = GetScaleFactor(lb->first.count(), ub->first.count(), l_time.count());
			return glm::mix(lb->second, ub->second, factor);
		}

		glm::quat slerpRotation(duration l_time, const AnimationChannels& chnls)
		{
			auto [lb,ub] = chnls.rotations.equal_range(l_time);

			if (lb != chnls.rotations.begin())
				lb--;

			double factor = GetScaleFactor(lb->first.count(), ub->first.count(), l_time.count());

			return glm::slerp(lb->second, ub->second, static_cast<float>(factor));
		}

		glm::vec3 mixScale(duration l_time, const AnimationChannels& chnls)
		{
			auto [lb,ub] = chnls.scales.equal_range(l_time);

			if (lb != chnls.scales.begin())
				lb--;

			double factor = GetScaleFactor(lb->first.count(), ub->first.count(), l_time.count());

			return glm::mix(lb->second, ub->second, factor);
		}
	};
}
