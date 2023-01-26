#pragma once
#include "Main/Scene.h"

namespace dmbrn
{
	class SceneTree
	{
	public:
		SceneTree(Scene& scene):
			scene_(scene),
			selected_(scene_.registry_)
		{
		}

		void newImGuiFrame()
		{
			ImGui::Begin("Scene Tree");

			auto& root_relation = scene_.scene_root_.getComponent<RelationshipComponent>();
			auto cur_id = root_relation.first;

			while (cur_id != entt::null)
			{
				recursivelyDraw(Enttity{scene_.registry_, cur_id});

				cur_id = scene_.registry_.get<RelationshipComponent>(cur_id).next;
			}

			if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
				selected_ = {scene_.registry_};

			if (ImGui::BeginPopupContextWindow(nullptr, 1))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
					scene_.addNewEntity("Empty Entity");

				ImGui::EndPopup();
			}

			ImGui::End();
		}

		Enttity& getSelected()
		{
			return selected_;
		}

	private:
		Scene& scene_;
		Enttity selected_;

		void recursivelyDraw(Enttity enttity)
		{
			const auto& tag = enttity.getComponent<TagComponent>();
			auto relation_comp = enttity.getComponent<RelationshipComponent>();

			ImGuiTreeNodeFlags flags =
				(selected_ == enttity ? ImGuiTreeNodeFlags_Selected : 0) |
				(relation_comp.first == entt::null ? ImGuiTreeNodeFlags_Leaf : 0) |
				ImGuiTreeNodeFlags_OpenOnArrow;

			bool opened = ImGui::TreeNodeEx(
				reinterpret_cast<const void*>(static_cast<uint64_t>(static_cast<uint32_t>(enttity))),
				flags, tag.tag.c_str());

			if (ImGui::IsItemClicked())
			{
				if (selected_)
				{
					if (ModelComponent* model_comp = selected_.tryGetComponent<ModelComponent>())
					{
						model_comp->shader_ = &Renderer::un_lit_textured;
					}
				}

				selected_ = enttity;

				if (ModelComponent* model_comp = enttity.tryGetComponent<ModelComponent>())
				{
					model_comp->shader_ = &Renderer::outlined_;
				}
			}

			if (opened)
			{
				auto cur_ind = relation_comp.first;
				while (cur_ind != entt::null)
				{
					recursivelyDraw(Enttity{scene_.registry_, cur_ind});

					cur_ind = scene_.registry_.get<RelationshipComponent>(cur_ind).next;
				}

				ImGui::TreePop();
			}
		}

		void drawEntityNode(Enttity& enttity)
		{
			const auto& tag = enttity.getComponent<TagComponent>();

			ImGuiTreeNodeFlags flags = (selected_ == enttity ? ImGuiTreeNodeFlags_Selected : 0) |
				ImGuiTreeNodeFlags_Leaf; //|	ImGuiTreeNodeFlags_OpenOnArrow
			bool opened = ImGui::TreeNodeEx(
				reinterpret_cast<const void*>(static_cast<uint64_t>(static_cast<uint32_t>(enttity))),
				flags, tag.tag.c_str());

			if (ImGui::IsItemClicked())
			{
				if (selected_)
				{
					if (ModelComponent* model_comp = enttity.tryGetComponent<ModelComponent>())
					{
						model_comp->shader_ = &Renderer::un_lit_textured;
					}
				}
				selected_ = enttity;
			}

			if (ModelComponent* model_comp = enttity.tryGetComponent<ModelComponent>())
			{
				model_comp->shader_ = &Renderer::outlined_;
			}

			bool deletedEntity = false;
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Delete Entity"))
				{
					deletedEntity = true;
					if (selected_ == enttity)
						selected_ = {scene_.registry_};
				}
				ImGui::EndPopup();
			}

			if (opened)
			{
				ImGui::TreePop();
			}

			if (deletedEntity)
				scene_.deleteEntity(enttity);
		}
	};
}
