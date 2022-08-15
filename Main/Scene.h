#pragma once

#include <entt/entt.hpp>
#include "Enttity.h"

namespace dmbrn
{
	class Scene
	{
	public:
		Scene(const Singletons& singletons, const ViewportRenderPass& render_pass):
			first(registry_, "First Barrel")
		{
			first.addComponent<MeshRendererComponent>("Models\\Barrel\\barell.obj", singletons, render_pass);
		}

		void draw(int curentFrame, const LogicalDevice& device, const vk::raii::CommandBuffer& command_buffer)
		{
			first.getComponent<MeshRendererComponent>().draw(curentFrame, device, command_buffer,first.getComponent<TransformComponent>());
		}

	private:
		entt::registry registry_;
		Enttity first;
	};
}
