#pragma once

#include "Componenets/AnimationComponent.h"

namespace dmbrn
{
	struct AnimationSequence
	{
		int mFrameMin, mFrameMax;
		std::unordered_map<Enttity, std::multimap<uint32_t, AnimationClip*>, Enttity::hash> entries_;

		int getAnimationComponentCount()
		{
			return entries_.size();
		}

		// TODO poor interface
		std::string getItemLabel(int i)
		{
			int ind = 0;
			for (auto it = entries_.begin(); it != entries_.end(); ++it, ++ind)
			{
				if (ind == i)
					return it->first.getComponent<TagComponent>().tag;
			}
		}

		Enttity getEnttity(int i)
		{
			int ind = 0;
			for (auto it = entries_.begin(); it != entries_.end(); ++it)
			{
				if (ind == i)
					return it->first;
			}
		}

		size_t getClipCount(Enttity ent)
		{
			return entries_[ent].size();
		}

		// TODO poor interface
		std::pair<int, int> getStartEnd(Enttity ent, int j)
		{
			int ind = 0;
			for (auto it = entries_[ent].begin(); it != entries_[ent].end(); ++it, ++ind)
			{
				// TODO remove casts
				if (ind == j)
					return {static_cast<int>(it->first), static_cast<int>(it->first + it->second->duration_)};
			}
		}

		std::pair<int, int> getStartEnd(int i, int j)
		{
			int indi = 0;
			for (auto iti = entries_.begin(); iti != entries_.end(); ++iti)
			{
				if (indi == i)
				{
					int indj = 0;
					for (auto itj = iti->second.begin(); itj != iti->second.end(); ++itj, ++indj)
					{
						// TODO remove casts
						if (indj == j)
							return {
								static_cast<int>(itj->first), static_cast<int>(itj->first + itj->second->duration_)
							};
					}
				}
			}
		}

		void updateStart(int i, int j, uint32_t new_start)
		{
			int indi = 0;
			for (auto iti = entries_.begin(); iti != entries_.end(); ++iti)
			{
				if (indi == i)
				{
					int indj = 0;
					for (auto itj = iti->second.begin(); itj != iti->second.end(); ++itj, ++indj)
					{
						// TODO remove casts
						if (indj == j)
						{
							AnimationClip* animation_clip = itj->second;
							entries_[iti->first].erase(itj->first);
							entries_[iti->first].insert({new_start, animation_clip});
							return;
						}
					}
				}
			}
		}

		std::string getClipName(int i, int j)
		{
			int indi = 0;
			for (auto iti = entries_.begin(); iti != entries_.end(); ++iti)
			{
				if (indi == i)
				{
					int indj = 0;
					for (auto itj = iti->second.begin(); itj != iti->second.end(); ++itj, ++indj)
					{
						// TODO remove casts
						if (indj == j)
							return itj->second->name;
					}
				}
			}
		}

	};
}
