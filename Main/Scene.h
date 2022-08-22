#pragma once

#include <entt/entt.hpp>
#include "Enttity.h"
#include "Wrappers/Model.h"
#include "Wrappers/Singletons/Renderer.h"

namespace dmbrn
{
	class Scene
	{
		friend class SceneTree;
	public:
		Scene(ImVec2 size):
			barrel(registry_, "First Barrel"),
			barrel2(registry_, "Barrel 2"),
			floor(registry_, "Floor"),
			camera(registry_, "Main Camera")
		{
			barrel.addComponent<ModelComponent>("Models\\Barrel\\barell.obj");
			barrel.addComponent<UnlitTextureMaterial>(Renderer::createUnlitTexturedMaterial());

			barrel2.addComponent<ModelComponent>("Models\\Barrel\\barell.obj");
			barrel2.addComponent<UnlitTextureMaterial>(Renderer::createUnlitTexturedMaterial());
			barrel2.getComponent<TransformComponent>().translate({0, -2, 0});

			floor.addComponent<ModelComponent>("Models\\GrassPlane\\grassPlane.obj");
			floor.addComponent<UnlitTextureMaterial>(Renderer::createUnlitTexturedMaterial());

			TransformComponent& floor_trans = floor.getComponent<TransformComponent>();

			floor_trans.translate({0, 0.7, 0});
			floor_trans.rotate({180, 0, 0});
			floor_trans.scale = {10, 10, 10};

			camera.getComponent<TransformComponent>().translate({-5, 0, 0});
			camera.addComponent<CameraComponent>(camera.getComponent<TransformComponent>(), size);
		}

		void changeCameraAspect(ImVec2 size)
		{
			camera.getComponent<CameraComponent>().changeAspect(size);
		}

		/**
		 * \brief should happen inside ImGui window model rendering (between begin and end), because uses ImGui window input events
		 */
		void update(float delta_t)
		{
			camera.getComponent<CameraComponent>().update(delta_t);
		}

		void draw(int curentFrame, const LogicalDevice& device, const vk::raii::CommandBuffer& command_buffer)
		{
			CameraComponent& camera_component = camera.getComponent<CameraComponent>();

			auto group = registry_.group<ModelComponent, UnlitTextureMaterial>(entt::get<TransformComponent>);
			Renderer::BeginUnlitTextureMaterial(command_buffer);
			for (auto entity : group)
			{
				auto [model,material,transform] = group.get<
					ModelComponent, UnlitTextureMaterial, TransformComponent>(entity);
				material.updateUBO(curentFrame, transform.getMatrix(), camera_component.getViewMat(),
				                   camera_component.proj_);
				model.draw(curentFrame, device, command_buffer, material);
			}
		}

	private:
		entt::registry registry_;
		Enttity barrel;
		Enttity barrel2;
		Enttity floor;
		Enttity camera;
	};
}
