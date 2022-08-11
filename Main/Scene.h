#pragma once

#include <entt/entt.hpp>
#include "Shape.h"

namespace dmbrn
{
	class Scene
	{
	public:
		Scene(const Singletons& singletons)
		{

		}
		
		void draw()
		{
			
		}

	private:
		entt::registry registry_;
	};
}
