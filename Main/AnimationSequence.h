#pragma once

#include "Componenets/AnimationComponent.h"

namespace dmbrn
{
	struct AnimationSequence
	{
		int mFrameMin, mFrameMax;
		std::unordered_map<Enttity, std::multimap<uint32_t, AnimationClip*>, Enttity::hash> entries_;

		using EntityIterator = decltype(entries_)::iterator;
		using ClipIterator = decltype(entries_)::mapped_type::iterator;

		int getAnimationComponentCount()
		{
			return entries_.size();
		}

		EntityIterator begin()
		{
			return entries_.begin();
		}

		EntityIterator end()
		{
			return entries_.end();
		}

		ClipIterator updateStart(EntityIterator ent_it, ClipIterator clip_it, uint32_t new_start)
		{
			AnimationClip* animation_clip = clip_it->second;
			ent_it->second.erase(clip_it);
			return {ent_it->second.insert({new_start,animation_clip})};
		}
	};
}
