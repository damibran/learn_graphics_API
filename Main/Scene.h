#pragma once

#include <entt/entt.hpp>
#include "Enttity.h"

namespace dmbrn
{
	class Scene
	{
	public:
		Scene(const Singletons& singletons, const ViewportRenderPass& render_pass):
			first(registry_,"First Barrel")
		{
			first.addComponent<MeshRendererComponent>("Models\\Barrel\\barell.obj",singletons,render_pass);
		}

		void draw()
		{
			
		}

	private:
		Enttity first;
		entt::registry registry_;
	};
}
