#pragma once

#include <entt/entt.hpp>
#include "Enttity.h"

namespace dmbrn
{
	class Scene
	{
	public:
		Scene(const Singletons& singletons, const ViewportRenderPass& render_pass, const bool& viewportFocused,
		      ImVec2 size):
			first(registry_, "First Barrel"),
			camera(registry_, "Main Camera")
		{
			first.addComponent<MeshRendererComponent>("Models\\Barrel\\barell.obj", singletons, render_pass);

			camera.getComponent<TransformComponent>().translate({-2,0,0});
			camera.addComponent<CameraComponent>(viewportFocused, camera.getComponent<TransformComponent>(), size);
		}

		void changeCameraAspect(ImVec2 size)
		{
			camera.getComponent<CameraComponent>().changeAspect(size);
		}

		void update(float delta_t)
		{
			camera.getComponent<CameraComponent>().update(delta_t);
		}

		void draw(int curentFrame, const LogicalDevice& device, const vk::raii::CommandBuffer& command_buffer)
		{
			CameraComponent& camera_component = camera.getComponent<CameraComponent>();
			first.getComponent<MeshRendererComponent>().draw(curentFrame, device, command_buffer,
			                                                 first.getComponent<TransformComponent>(),
			                                                 camera_component.getViewMat(), camera_component.proj_);
		}

	private:
		entt::registry registry_;
		Enttity first;
		Enttity camera;
	};
}
