#pragma once

#include <entt/entt.hpp>
#include "Shape.h"
#include "Enttity.h"

namespace dmbrn
{
	class Scene
	{
	public:
		Scene(const Singletons& singletons):
			first(registry_,"First Barrel")
		{

		}
		
		void draw()
		{
			
		}

	private:
		Enttity first;
		entt::registry registry_;
	};
}
