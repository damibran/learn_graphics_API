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
			barrel(registry_, "First Barrel"),
			floor(registry_,"Floor"),
			camera(registry_, "Main Camera")
		{
			barrel.addComponent<MeshRendererComponent>("Models\\Barrel\\barell.obj", singletons, render_pass);
			floor.addComponent<MeshRendererComponent>("Models\\GrassPlane\\grassPlane.obj",singletons,render_pass);

			TransformComponent& floor_trans = floor.getComponent<TransformComponent>();

			floor_trans.translate({0,-0.7/10,0});
			floor_trans.rotate({180,0,0});
			floor_trans.scale = {10,10,10};

			camera.getComponent<TransformComponent>().translate({-5,0,0});
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

			auto group = registry_.group<MeshRendererComponent>(entt::get<TransformComponent>);
			for (auto entity : group)
			{
				auto [model,transform] = group.get<MeshRendererComponent,TransformComponent>(entity);
				model.draw(curentFrame, device, command_buffer,
			                                                 transform,
			                                                 camera_component.getViewMat(), camera_component.proj_);
			}
		}

	private:
		entt::registry registry_;
		Enttity barrel;
		Enttity floor;
		Enttity camera;
	};
}
