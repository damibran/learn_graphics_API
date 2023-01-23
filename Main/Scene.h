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
			SceneNode root = ModelImporter::Import(path);

			recursivelyAdd(root);			
		}

		void recursivelyAdd(const SceneNode& node)
		{
			Enttity ent{registry_, node.name};

			if(node.mesh)
				ent.addComponent<ModelComponent>(node.mesh,&Renderer::un_lit_textured );

			TransformComponent& t = ent.getComponent<TransformComponent>();
			t.position = node.transform.position;
			t.rotation = node.transform.rotation;
			t.scale = node.transform.scale;

			for (const auto & child : node.children)
			{
				recursivelyAdd(child);
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
