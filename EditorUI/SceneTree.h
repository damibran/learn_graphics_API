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
				ImGuiTreeNodeFlags_OpenOnArrow;
			bool opened = ImGui::TreeNodeEx(reinterpret_cast<const void*>(static_cast<uint64_t>(enttity.getId())), flags, tag.tag.c_str());

			if (ImGui::IsItemClicked())
			{
				selected_ = enttity;
			}

			if (opened)
			{
				ImGui::TreePop();
			}
		}
	};
}
