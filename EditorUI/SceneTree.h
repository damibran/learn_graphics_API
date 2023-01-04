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

			scene_.registry_.each([&](auto entityId)
			{
				Enttity entity{scene_.registry_, entityId};
				drawEntityNode(entity);
			});

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

		void drawEntityNode(const Enttity& enttity)
		{
			const auto& tag = enttity.getComponent<TagComponent>();

			ImGuiTreeNodeFlags flags = (selected_ == enttity ? ImGuiTreeNodeFlags_Selected : 0) |
				ImGuiTreeNodeFlags_Leaf; //|	ImGuiTreeNodeFlags_OpenOnArrow
			bool opened = ImGui::TreeNodeEx(
				reinterpret_cast<const void*>(static_cast<uint64_t>(static_cast<uint32_t>(enttity))),
				flags, tag.tag.c_str());

			if (ImGui::IsItemClicked())
			{
				selected_ = enttity;
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
