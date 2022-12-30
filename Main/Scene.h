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
		Scene(ImVec2 size)//:
			//camera(registry_, "Main Camera")
		{
			Enttity barrel{registry_, "Barrel 1"};
			Enttity barrel2{registry_, "Barrel 2"};
			Enttity floor{registry_, "Floor"};
			barrel.addComponent<ModelComponent>("Models\\Barrel\\barrel.dae");

			barrel2.addComponent<ModelComponent>("Models\\Barrel\\barrel.dae");
			barrel2.getComponent<TransformComponent>().translate({0, -2, 0});

			floor.addComponent<ModelComponent>("Models\\GrassPlane\\grassPlane.dae");

			TransformComponent& floor_trans = floor.getComponent<TransformComponent>();

			floor_trans.translate({0, 0.7, 0});
			floor_trans.rotate({-180, 0, 0});
			//floor_trans.scale = {10, 10, 10};

			//frame.addComponent<ModelComponent>("Models\\Frame\\Frame.dae");

			//camera.getComponent<TransformComponent>().translate({0, 0, -7});
			//camera.addComponent<CameraComponent>(camera.getComponent<TransformComponent>(), size);
		}

		void addNewEntity(const std::string& name = std::string{})
		{
			Enttity enttity{registry_, name};
		}

		void deleteEntity(Enttity enttity)
		{
			registry_.destroy(enttity);
		}

		auto getModelsToDraw()
		{
			return registry_.group<ModelComponent>(entt::get<TransformComponent>);
		}

	private:
		entt::registry registry_;
	};
}
