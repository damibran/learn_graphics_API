#pragma once
#include <string>

namespace dmbrn
{
	struct TagComponent
	{
		std::string tag;

		TagComponent(const std::string& name) :
			tag(name)
		{
		}
	};
}
