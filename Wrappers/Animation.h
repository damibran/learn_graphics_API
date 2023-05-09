#pragma once

#include<glm/glm.hpp>
#include <set>
#include <string>
#include <algorithm>

#include "Main/Enttity.h"

namespace dmbrn
{
	struct AnimationChannels
	{
		std::map<float, glm::vec3> positions;
		std::map<float, glm::quat> rotations;
		std::map<float, glm::vec3> scales;
	};

	struct AnimationClip
	{
		std::string name;
		float duration_; // in frames
		std::unordered_map<Enttity, AnimationChannels,Enttity::hash> channels;

		void updateTransforms(float cur_local_frame, uint32_t frame)
		{
			for (auto& chnls : channels)
			{
				TransformComponent& ent_trans_c = chnls.first.getComponent<TransformComponent>();

				if (!chnls.second.positions.empty())
					ent_trans_c.position = mixPositions(cur_local_frame, chnls.second);
				if (!chnls.second.rotations.empty())
					ent_trans_c.setQuat(slerpRotation(cur_local_frame, chnls.second));
				if (!chnls.second.scales.empty())
					ent_trans_c.scale = mixScale(cur_local_frame, chnls.second);

				chnls.first.markTransformAsEdited(frame);
			}
		}

		bool operator<(const AnimationClip& other) const
		{
			return std::lexicographical_compare(name.begin(), name.end(), other.name.begin(), other.name.end());
		}

	private:
		double GetScaleFactor(double lastTimeStamp, double nextTimeStamp, double animationTime)
		{
			const double midWayLength = animationTime - lastTimeStamp;
			const double framesDiff = nextTimeStamp - lastTimeStamp;
			return midWayLength / framesDiff;
		}

		glm::vec3 mixPositions(float l_time, const AnimationChannels& chnls)
		{
			auto [lb,ub] = chnls.positions.equal_range(l_time);

			if (lb != chnls.positions.begin())
				--lb;

			if (ub == chnls.positions.end())
				--ub;

			const double factor = GetScaleFactor(lb->first, ub->first, l_time);
			return glm::mix(lb->second, ub->second, factor);
		}

		glm::quat slerpRotation(float l_time, const AnimationChannels& chnls)
		{
			auto [lb,ub] = chnls.rotations.equal_range(l_time);

			if (lb != chnls.rotations.begin())
				--lb;

			if (ub == chnls.rotations.end())
				--ub;

			const double factor = GetScaleFactor(lb->first, ub->first, l_time);

			return glm::slerp(lb->second, ub->second, static_cast<float>(factor));
		}

		glm::vec3 mixScale(float l_time, const AnimationChannels& chnls)
		{
			auto [lb,ub] = chnls.scales.equal_range(l_time);

			if (lb != chnls.scales.begin())
				--lb;

			if (ub == chnls.scales.end())
				--ub;

			const double factor = GetScaleFactor(lb->first, ub->first, l_time);

			return glm::mix(lb->second, ub->second, factor);
		}
	};
}
