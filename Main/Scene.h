#pragma once

#include <entt/entt.hpp>
#include "Enttity.h"
#include "Wrappers/Singletons/Renderer.h"
#include "Wrappers/ModelImporter.h"

namespace dmbrn
{
	// need interface implementation for scene tree
	class Scene
	{
		friend class SceneTree;
	public:
		Scene():
			scene_root_(registry_, "SceneRoot")
		{
			scene_root_.addComponent<RelationshipComponent>();

			addModel("Models\\Char\\Warrok W Kurniawan.fbx");
			//addModel("Models\\DoubleTestCube\\DoubleTestCube.fbx");
		}

		void addNewEntity(const std::string& name = std::string{})
		{
			Enttity enttity{registry_, name};
		}

		void deleteEntity(Enttity& enttity)
		{
			registry_.destroy(enttity);
		}

		void addModel(const std::string& path)
		{
			SceneNode root = ModelImporter::Import(path);

			recursivelyAddTo(root, scene_root_);
		}

		void recursivelyAddTo(SceneNode& node, Enttity& parent)
		{
			Enttity child_ent{registry_, node.name, parent};

			if (node.mesh.render_data_)
				child_ent.addComponent<ModelComponent>(std::move(node.mesh), &Renderer::un_lit_textured);

			TransformComponent& t = child_ent.getComponent<TransformComponent>();
			t.position = node.transform.position;
			t.rotation = node.transform.rotation;
			t.scale = node.transform.scale;

			for (auto& child : node.children)
			{
				recursivelyAddTo(child, child_ent);
			}
		}

		// for now updates data for all entities
		void updatePerObjectData(uint32_t frame)
		{
			auto group = registry_.group<ModelComponent>(entt::get<TransformComponent>);

			char* data = ModelComponent::per_object_data_buffer_.map(frame);

			recursivelyUpdateMatrix(scene_root_, glm::mat4{1.0f}, data);

			ModelComponent::per_object_data_buffer_.unMap(frame);
		}

		void recursivelyUpdateMatrix(const Enttity& ent, const glm::mat4& parent_matrix, char* data_map)
		{
			const TransformComponent& this_trans = ent.getComponent<TransformComponent>();
			glm::mat4 this_matrix = parent_matrix * this_trans.getMatrix();

			if (const ModelComponent* model = ent.tryGetComponent<ModelComponent>())
			{
				auto ubo_data = reinterpret_cast<PerObjectDataBuffer::UBODynamicData*>(data_map + model->
					inGPU_transform_offset);
				ubo_data->model = this_matrix;
			}

			auto& cur_comp = ent.getComponent<RelationshipComponent>();
			auto cur_id = cur_comp.first;

			while (cur_id != entt::null)
			{
				recursivelyUpdateMatrix(Enttity{registry_, cur_id}, this_matrix, data_map);

				cur_id = registry_.get<RelationshipComponent>(cur_id).next;
			}
		}

		// may perform culling
		auto getModelsToDraw()
		{
			return registry_.group<ModelComponent>(entt::get<TransformComponent>);
		}

	private:
		entt::registry registry_;
		Enttity scene_root_;
	};
}
