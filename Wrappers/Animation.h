#pragma once

#include<glm/glm.hpp>
#include <set>
#include <string>
#include <algorithm>
#include <variant>

#include "Main/Enttity.h"

namespace dmbrn
{
	struct AnimationChannels
	{
		struct PosKeyTag;
		struct RotKeyTag;
		struct ScaleKeyTag;

		std::map<float, glm::vec3> positions;
		std::map<float, glm::quat> rotations;
		std::map<float, glm::vec3> scales;

		template <typename KeyTag, typename KeyType>
		inline void setKey(float time, const KeyType& value)
		{
			assert(false);
		}

		template <>
		inline void setKey<PosKeyTag, glm::vec3>(float time, const glm::vec3& value)
		{
			positions[time] = value;
		}

		template <>
		inline void setKey<RotKeyTag, glm::quat>(float time, const glm::quat& value)
		{
			rotations[time] = value;
		}

		template <>
		inline void setKey<ScaleKeyTag, glm::vec3>(float time, const glm::vec3& value)
		{
			scales[time] = value;
		}

		float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float loc_time)
		{
			const float midWayLength = loc_time - lastTimeStamp;
			const float framesDiff = nextTimeStamp - lastTimeStamp;
			return midWayLength / framesDiff;
		}

		glm::vec3 mixPositions(float l_time)
		{
			auto [lb,ub] = positions.equal_range(l_time);

			if(lb==positions.end())
				lb = --positions.end();

			if(lb!=positions.begin() && lb->first > l_time)
				--lb;

			if (ub == positions.end())
				--ub;

			float factor = 0.f;
			if (lb != ub)
				factor = GetScaleFactor(lb->first, ub->first, l_time);
			else
				factor = 0;

			return glm::mix(lb->second, ub->second, factor);
		}

		glm::quat slerpRotation(float l_time)
		{
			auto [lb,ub] = rotations.equal_range(l_time);

			if(lb==rotations.end())
				lb = --rotations.end();

			if(lb!=rotations.begin()&& lb->first > l_time)
				--lb;

			if (ub == rotations.end())
				--ub;

			float factor = 0.f;
			if (lb != ub)
				factor = GetScaleFactor(lb->first, ub->first, l_time);
			else
				factor = 0;

			return glm::slerp(lb->second, ub->second, factor);
		}

		glm::vec3 mixScale(float l_time)
		{
			auto [lb,ub] = scales.equal_range(l_time);

			if(lb==scales.end())
				lb = --scales.end();

			if(lb!=scales.begin() && lb->first > l_time)
				--lb;

			if (ub == scales.end())
				--ub;

			float factor = 0.f;
			if (lb != ub)
				factor = GetScaleFactor(lb->first, ub->first, l_time);
			else
				factor = 0;

			return glm::mix(lb->second, ub->second, factor);
		}

	};

	struct AnimationClip
	{
		std::string name;
		float min = 0.f, max = 0.f;
		std::unordered_map<Enttity, AnimationChannels, Enttity::hash> channels;

		void updateTransforms(float cur_local_frame, uint32_t frame)
		{
			for (auto& chnls : channels)
			{
				TransformComponent& ent_trans_c = chnls.first.getComponent<TransformComponent>();

				if (!chnls.second.positions.empty())
					ent_trans_c.position = chnls.second.mixPositions(cur_local_frame);
				if (!chnls.second.rotations.empty())
					ent_trans_c.setQuat(chnls.second.slerpRotation(cur_local_frame));
				if (!chnls.second.scales.empty())
					ent_trans_c.scale = chnls.second.mixScale(cur_local_frame);

				chnls.first.markTransformAsEdited(frame);
			}
		}

		bool operator<(const AnimationClip& other) const
		{
			return std::lexicographical_compare(name.begin(), name.end(), other.name.begin(), other.name.end());
		}

		float getDuration() const
		{
			return max - min;
		}
	};
}
