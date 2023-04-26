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
		std::map<uint32_t, glm::vec3> positions;
		std::map<uint32_t, glm::quat> rotations;
		std::map<uint32_t, glm::vec3> scales;
	};

	struct AnimationClip
	{
		std::string name;
		uint32_t duration_; // in frames
		std::vector<AnimationChannels> channels;

		void updateTransforms(float cur_local_frame, uint32_t frame)
		{
			for (auto& chnls : channels)
			{
				TransformComponent& ent_trans_c = chnls.enttity.getComponent<TransformComponent>();
		
				ent_trans_c.position = mixPositions(cur_local_frame, chnls);
				ent_trans_c.setQuat(slerpRotation(cur_local_frame, chnls));
				ent_trans_c.scale = mixScale(cur_local_frame, chnls);
		
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

		glm::vec3 mixPositions(float l_time, const AnimationChannels& chnls)
		{
			auto [lb,ub] = chnls.positions.equal_range(l_time);
		
			if (lb != chnls.positions.begin())
				--lb;

			if(ub == chnls.positions.end())
				--ub;

			double factor = GetScaleFactor(lb->first, ub->first, l_time);
			return glm::mix(lb->second, ub->second, factor);
		}
		
		glm::quat slerpRotation(float l_time, const AnimationChannels& chnls)
		{
			auto [lb,ub] = chnls.rotations.equal_range(l_time);
		
			if (lb != chnls.rotations.begin())
				--lb;

			if(ub == chnls.rotations.end())
				--ub;
		
			double factor = GetScaleFactor(lb->first, ub->first, l_time);
		
			return glm::slerp(lb->second, ub->second, static_cast<float>(factor));
		}
		
		glm::vec3 mixScale(float l_time, const AnimationChannels& chnls)
		{
			auto [lb,ub] = chnls.scales.equal_range(l_time);
		
			if (lb != chnls.scales.begin())
				--lb;

			if(ub == chnls.scales.end())
				--ub;
		
			double factor = GetScaleFactor(lb->first, ub->first, l_time);
		
			return glm::mix(lb->second, ub->second, factor);
		}
	};
}
