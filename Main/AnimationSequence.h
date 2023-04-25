#pragma once

#include "Componenets/AnimationComponent.h"

namespace dmbrn
{
	struct AnimationSequence
	{
		int mFrameMin, mFrameMax;
		std::unordered_map<Enttity, std::multimap<uint32_t, AnimationClip*>, Enttity::hash> entries_;

		struct EntityIterator
		{
			// may be delete this class
			struct ClipIterator
			{
				bool operator==(const ClipIterator& other) const
				{
					return clip_iter == other.clip_iter;
				}

				bool operator!=(const ClipIterator& other) const
				{
					return !(*this == other);
				}

				ClipIterator& operator++()
				{
					++clip_iter;
					return *this;
				}

				std::pair<int,int> getStartEnd()
				{
					return {static_cast<int>(clip_iter->first), static_cast<int>(clip_iter->first + clip_iter->second->duration_)};
				}

				std::multimap<uint32_t, AnimationClip*>::pointer operator->()
				{
					return &*clip_iter;
				}

				std::multimap<uint32_t, AnimationClip*>::iterator operator*()
				{
					return clip_iter;
				}

				std::multimap<uint32_t, AnimationClip*>::iterator clip_iter;
			};

			bool operator==(const EntityIterator& other) const
			{
				return ent_iter == other.ent_iter;
			}

			bool operator!=(const EntityIterator& other) const
			{
				return !(*this == other);
			}

			EntityIterator& operator++()
			{
				++ent_iter;
				++ind;

				return *this;
			}

			ClipIterator begin()
			{
				return ClipIterator{ent_iter->second.begin()};
			}

			ClipIterator end()
			{
				return {ent_iter->second.end()};
			}

			decltype(entries_)::pointer operator->()
				{
					return &*ent_iter;
				}

			decltype(entries_)::iterator ent_iter;
			int ind=0;
		};

		int getAnimationComponentCount()
		{
			return entries_.size();
		}

		EntityIterator begin()
		{
			return EntityIterator{entries_.begin()};
		}

		EntityIterator end()
		{
			return EntityIterator{entries_.end()};
		}

		EntityIterator::ClipIterator updateStart(EntityIterator ent_it, EntityIterator::ClipIterator clip_it, uint32_t new_start)
		{
			AnimationClip* animation_clip = clip_it->second;
			ent_it->second.erase(*clip_it);
			return {ent_it->second.insert({new_start,animation_clip})};
		}
	};
}
