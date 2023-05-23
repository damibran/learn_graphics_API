#pragma once
#include <string>

namespace dmbrn
{
	/**
	 * \brief name of an entity
	 */
	struct TagComponent
	{
		std::string tag;

		TagComponent(const std::string& name) :
			tag(name)
		{
		}
	};
}
