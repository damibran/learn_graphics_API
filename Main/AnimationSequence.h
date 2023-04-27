#pragma once

namespace dmbrn
{
	class AnimationSequence
	{
		// TODO make private
	public:
		std::unordered_map<Enttity, std::multimap<float, AnimationClip>, Enttity::hash> entries_;
	public:
		int mFrameMin, mFrameMax;
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

		ClipIterator updateStart(const EntityIterator& ent_it, ClipIterator&& clip_it, float new_start)
		{
			AnimationClip animation_clip = std::move(clip_it->second);
			ent_it->second.erase(clip_it);
			return {ent_it->second.insert({new_start, animation_clip})};
		}
	};
}
