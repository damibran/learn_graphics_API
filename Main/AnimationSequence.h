#pragma once

#include <variant>
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

		template <typename KeyTag, typename KeyType>
		[[nodiscard]] ClipIterator updateClipWithKey(const Enttity& rec_parent, Enttity trans_ent, float currentFrame,
		                                             std::move_iterator<ClipIterator> sel_clip, const KeyType& key)
		{
			float start = sel_clip->first;
			float end = sel_clip->first + sel_clip->second.getDuration();
			if (currentFrame >= start && currentFrame <= end)
			{
				sel_clip->second.channels[trans_ent].setKey<KeyTag>(currentFrame - start + sel_clip->second.min, key);
				return sel_clip.base();
			}
			else if (currentFrame > end)
			{
				sel_clip->second.channels[trans_ent].setKey<KeyTag>(currentFrame - start + sel_clip->second.min, key);
				sel_clip->second.max = currentFrame - start + sel_clip->second.min;
				return sel_clip.base();
			}
			else if (currentFrame < start)
			{
				sel_clip->second.channels[trans_ent].setKey<KeyTag>(currentFrame - start + sel_clip->second.min, key);
				sel_clip->second.min = currentFrame - start + sel_clip->second.min;
				return updateStart(entries_.find(rec_parent), sel_clip, currentFrame);
			}
		}

		template <typename KeyTag, typename KeyType>
		[[nodiscard]] ClipIterator createNewClipWithKey(const Enttity& rec_parent, const Enttity* enttity,
		                                                float key_time, const KeyType& key)
		{
			decltype(AnimationClip::channels) channels;
			channels[*enttity].setKey<KeyTag>(key_time, key);
			AnimationClip new_clip{"New clip", key_time, key_time, std::move(channels)};
			return entries_[rec_parent].insert({key_time, std::move(new_clip)});
		}
	};
}
