#pragma once

#include "SceneTree.h"

namespace dmbrn
{
	class Inspector
	{
	public:
		Inspector(SceneTree& scene_tree):
			scene_tree_(scene_tree)
		{
		}

		void newImGuiFrame()
		{
			ImGui::Begin("Inspector");

			if (scene_tree_.getSelected())
			{
				Enttity& entity = scene_tree_.getSelected();

				drawComponents(entity);

				if (ImGui::Button("Add Component"))
					ImGui::OpenPopup("Add Component");

				if (ImGui::BeginPopup("Add Component"))
				{
					if (ImGui::MenuItem("Model Component"))
					{
						entity.addComponent<ModelComponent>("");
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
			}

			ImGui::End();
		}

	private:
		SceneTree& scene_tree_;

		void drawComponents(Enttity entity)
		{
			if (auto* comp = entity.tryGetComponent<TagComponent>())
			{
				char buf[256];
				memset(buf, 0, sizeof(buf));
				strcpy_s(buf, sizeof(buf), comp->tag.c_str());

				if (ImGui::InputText("Tag", buf, sizeof(buf)))
				{
					comp->tag = std::string(buf);
				}
			}

			if (auto* comp = entity.tryGetComponent<TransformComponent>())
			{
				if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
				{
					drawVec3Control("Translation", comp->position);
					drawVec3Control("Rotation", comp->rotation);
					drawVec3Control("Scale", comp->scale, 1.0f);
					ImGui::TreePop();
				}
			}

			if (auto* comp = entity.tryGetComponent<ModelComponent>())
			{
				if (ImGui::TreeNodeEx("ModelComponent", ImGuiTreeNodeFlags_DefaultOpen))
				{
					const Model* model = comp->getModel();
					std::string text = model ? model->getPath() : "";
					char buf[256];
					memset(buf, 0, sizeof(buf));
					strcpy_s(buf, sizeof(buf), text.c_str());
					bool newPath = false;
					ImGui::Text("Model: ");
					ImGui::SameLine();
					if (ImGui::InputText("##ModelPathLabel", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue))
					{
						comp->setNewModel(buf);
					}

					ImGui::TreePop();
				}
			}
		}

		static void drawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f,
		                            float columnWidth = 100.0f)
		{
			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];

			ImGui::PushID(label.c_str());

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, columnWidth);
			ImGui::Text(label.c_str());
			ImGui::NextColumn();

			ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
			ImGui::PushFont(boldFont);
			if (ImGui::Button("X", buttonSize))
				values.x = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
			ImGui::PushFont(boldFont);
			if (ImGui::Button("Y", buttonSize))
				values.y = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
			ImGui::PushFont(boldFont);
			if (ImGui::Button("Z", buttonSize))
				values.z = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();

			ImGui::PopStyleVar();

			ImGui::Columns(1);

			ImGui::PopID();
		}
	};
}
