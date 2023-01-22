#pragma once

#include <entt/entt.hpp>
#include "Enttity.h"
#include "Wrappers/Singletons/Renderer.h"
#include "Wrappers/ModelImporter.h"

namespace dmbrn
{
	class Scene
	{
		friend class SceneTree;
	public:
		Scene(ImVec2 size)
		{
			addModel("Models\\Double_Barrel\\Double_Barrel.fbx");
		}

		void addNewEntity(const std::string& name = std::string{})
		{
			Enttity enttity{registry_, name};
		}

		void deleteEntity(Enttity enttity)
		{
			registry_.destroy(enttity);
		}

		void addModel(const std::string& path)
		{
			auto import = ModelImporter::Import(path);

			std::string model_name = path.substr(path.find_last_of('\\') + 1,
			                                     path.find_last_of('.') - path.find_last_of('\\') - 1);

			for (const auto& pair : import)
			{
				Enttity ent{registry_, pair.first->name};

				ent.addComponent<ModelComponent>(pair.first,&Renderer::un_lit_textured);

				TransformComponent& t = ent.getComponent<TransformComponent>();
				t.position = pair.second.position;
				t.rotation = pair.second.rotation;
				t.scale = pair.second.scale;
			}
		}

		void updatePerObjectData(uint32_t frame)
		{
			auto group = registry_.group<ModelComponent>(entt::get<TransformComponent>);

			char* data = ModelComponent::per_object_data_buffer_.map(frame);

			for (auto entity : group)
			{
				auto [model,transform] = group.get<ModelComponent, TransformComponent>(entity);

				auto ubo_data = reinterpret_cast<PerObjectDataBuffer::UBODynamicData*>(data + model.
					inGPU_transform_offset);

				ubo_data->model = transform.getMatrix();
			}

			ModelComponent::per_object_data_buffer_.unMap(frame);
		}

		// may perform culling
		auto getModelsToDraw()
		{
			return registry_.group<ModelComponent>(entt::get<TransformComponent>);
		}

	private:
		entt::registry registry_;
	};
}
