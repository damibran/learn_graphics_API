#pragma once

#include<glm/glm.hpp>
#include <set>
#include <string>
#include <unordered_map>

#include "Main/Enttity.h"

namespace dmbrn
{
	struct AnimationChannels
	{
		Enttity enttity;
		std::map<double, glm::vec3> positions;
		std::map<double, glm::quat> rotations;
		std::map<double, glm::vec3> scales;
	};

	struct AnimationClip
	{
		std::string name;
		double duration; // in seconds
		std::vector<AnimationChannels> channels;

		void updateTransforms(double l_time, uint32_t frame)
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
		float GetScaleFactor(double lastTimeStamp, double nextTimeStamp, double animationTime)
		{
			float scaleFactor = 0.0f;
			float midWayLength = animationTime - lastTimeStamp;
			float framesDiff = nextTimeStamp - lastTimeStamp;
			scaleFactor = midWayLength / framesDiff;
			return scaleFactor;
		}

		glm::vec3 mixPositions(double l_time, const AnimationChannels& chnls)
		{
			const auto& [lb,ub] = chnls.positions.equal_range(l_time);

			float factor = GetScaleFactor(lb->first, ub->first, l_time);

			return glm::mix(lb->second, ub->second, factor);
		}

		glm::quat slerpRotation(double l_time, const AnimationChannels& chnls)
		{
			const auto& [lb,ub] = chnls.rotations.equal_range(l_time);

			float factor = GetScaleFactor(lb->first, ub->first, l_time);

			return glm::slerp(lb->second, ub->second, factor);
		}

		glm::vec3 mixScale(double l_time, const AnimationChannels& chnls)
		{
			const auto& [lb,ub] = chnls.scales.equal_range(l_time);

			float factor = GetScaleFactor(lb->first, ub->first, l_time);

			return glm::mix(lb->second, ub->second, factor);
		}
	};
}
