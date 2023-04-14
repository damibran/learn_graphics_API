#pragma once
#include "Main/Scene.h"

namespace dmbrn
{
	class SceneTree
	{
	public:
		SceneTree(Scene& scene):
			scene_(scene),
			selected_(scene_.getNullEntt())
		{
		}

		void newImGuiFrame()
		{
			ImGui::Begin("Scene Tree");

			Enttity cur_child = scene_.getRootRelationshipComponent().first;

			while (cur_child)
			{
				recursivelyDraw(cur_child);

				cur_child = cur_child.getComponent<RelationshipComponent>().next;
			}

			if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
			{
				if (selected_)
					deselect(selected_);
				selected_ = scene_.getNullEntt();
			}

			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::MenuItem("Create Empty Entity"))
					scene_.addNewEntityToRoot("Empty Entity");

				ImGui::EndPopup();
			}

			ImGui::End();
		}

		Enttity* getSelected()
		{
			return &selected_;
		}

	private:
		Scene& scene_;
		Enttity selected_;

		void recursivelyDraw(Enttity enttity)
		{
			const TagComponent& tag = enttity.getComponent<TagComponent>();
			const RelationshipComponent& relation_comp = enttity.getComponent<RelationshipComponent>();

			ImGuiTreeNodeFlags flags =
				(selected_ == enttity ? ImGuiTreeNodeFlags_Selected : 0) |
				(!relation_comp.first ? ImGuiTreeNodeFlags_Leaf : 0) |
				ImGuiTreeNodeFlags_OpenOnArrow;

			bool opened = ImGui::TreeNodeEx(
				reinterpret_cast<const void*>(static_cast<uint64_t>(static_cast<uint32_t>(enttity))),
				flags, tag.tag.c_str());

			if (ImGui::IsItemClicked())
			{
				if (selected_)
				{
					deselect(selected_);
				}

				selected_ = enttity;

				select(selected_);
			}

			if (opened)
			{
				Enttity cur_child = relation_comp.first;
				while (cur_child)
				{
					recursivelyDraw(cur_child);

					cur_child = cur_child.getComponent<RelationshipComponent>().next;
				}

				ImGui::TreePop();
			}
		}

		void deselect(Enttity enttity)
		{
			if (ModelComponent* model_comp = enttity.tryGetComponent<ModelComponent>())
			{
				model_comp->shader_ = &Renderer::un_lit_textured;
			}

			Enttity cur_child = enttity.getComponent<RelationshipComponent>().first;
			while (cur_child)
			{
				deselect(cur_child);

				cur_child = cur_child.getComponent<RelationshipComponent>().next;
			}
		}

		void select(Enttity enttity)
		{
			if (ModelComponent* model_comp = enttity.tryGetComponent<ModelComponent>())
			{
				//model_comp->shader_ = &Renderer::outlined_;
			}

			Enttity cur_child = enttity.getComponent<RelationshipComponent>().first;
			while (cur_child )
			{
				select(cur_child);

				cur_child = cur_child.getComponent<RelationshipComponent>().next;
			}
		}
	};
}
