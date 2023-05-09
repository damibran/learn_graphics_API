#pragma once

#include "Wrappers/Animation.h"

namespace dmbrn
{
	class AnimationSequence
	{
		// TODO make private
	public:
		std::unordered_map<Enttity, std::multimap<float, AnimationClip>, Enttity::hash> entries_;
	public:
		int mFrameMin = 0, mFrameMax = 0;
		using EntityIterator = decltype(entries_)::iterator;
		using ClipIterator = decltype(entries_)::mapped_type::iterator;

		int getAnimationComponentCount()
		{
			return static_cast<int>(entries_.size());
		}

		EntityIterator begin()
		{
			return entries_.begin();
		}

		EntityIterator end()
		{
			return entries_.end();
		}

		void add(Enttity ent, float start, AnimationClip clip)
		{
			entries_[ent].insert({start, clip});
		}

		[[nodiscard]] ClipIterator updateStart(const EntityIterator& ent_it, std::move_iterator<ClipIterator> clip_it,
		                         float new_start)
		{
			AnimationClip animation_clip = std::move(clip_it->second);
			ent_it->second.erase(clip_it.base());
			return {ent_it->second.insert({new_start, std::move(animation_clip)})};
		}

		[[nodiscard]] ClipIterator processPositionKey(const Enttity* enttity, const Enttity& rec_parent,const glm::vec3& key, float key_time)
		{
			decltype(AnimationClip::channels) channels;
			channels[enttity->getId()].positions[key_time] = key;
			channels[enttity->getId()].enttity=*enttity;
			AnimationClip new_clip{"New clip", 0, std::move(channels)};
			return entries_[rec_parent].insert({key_time, std::move(new_clip)});
		}

		[[nodiscard]] ClipIterator processRotationKey(const Enttity* enttity, const Enttity& rec_parent,const glm::quat& key, float key_time)
		{
			decltype(AnimationClip::channels) channels;
			channels[enttity->getId()].rotations[key_time] = key;
			channels[enttity->getId()].enttity=*enttity;
			AnimationClip new_clip{"New clip", 0, std::move(channels)};
			return entries_[rec_parent].insert({key_time, std::move(new_clip)});
		}

		[[nodiscard]] ClipIterator processScaleKey(const Enttity* enttity, const Enttity& rec_parent,const glm::vec3& key, float key_time)
		{
			decltype(AnimationClip::channels) channels;
			channels[enttity->getId()].scales[key_time] = key;
			channels[enttity->getId()].enttity=*enttity;
			AnimationClip new_clip{"New clip", 0, std::move(channels)};
			return entries_[rec_parent].insert({key_time, std::move(new_clip)});
		}
	};
}
