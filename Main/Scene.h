#pragma once

#include <entt/entt.hpp>
#include "Enttity.h"
#include "Wrappers/Model.h"
#include "Wrappers/Singletons/Renderer.h"

namespace dmbrn
{
	class Scene
	{
	public:
		Scene(const bool& viewportFocused,
		      ImVec2 size):
			barrel(registry_, "First Barrel"),
			floor(registry_, "Floor"),
			camera(registry_, "Main Camera")
		{
			barrel.addComponent<ModelComponent>("Models\\Barrel\\barell.obj");
			//barrel.addComponent<UnlitTextureMaterial>(std::move(Renderer::createUnlitTexturedMaterial()));

			floor.addComponent<ModelComponent>("Models\\GrassPlane\\grassPlane.obj");
			//floor.addComponent<UnlitTextureMaterial>(std::move(Renderer::createUnlitTexturedMaterial()));

			TransformComponent& floor_trans = floor.getComponent<TransformComponent>();

			floor_trans.translate({0, -0.7 / 10, 0});
			floor_trans.rotate({180, 0, 0});
			floor_trans.scale = {10, 10, 10};

			camera.getComponent<TransformComponent>().translate({-5, 0, 0});
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

			auto group = registry_.group<UnlitTextureMaterial>(entt::get<TransformComponent, Model>);
			Renderer::BeginUnlitTextureMaterial(command_buffer);
			for (auto entity : group)
			{
				auto [model,material,transform] = group.get<Model, UnlitTextureMaterial, TransformComponent>(entity);
				material.updateUBO(curentFrame, transform.getMatrix(), camera_component.getViewMat(),
				                   camera_component.proj_);
				model.draw(curentFrame, device, command_buffer, material);
			}
		}

	private:
		entt::registry registry_;
		Enttity barrel;
		Enttity floor;
		Enttity camera;
	};
}
