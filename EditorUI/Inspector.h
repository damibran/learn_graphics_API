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

		void newImGuiFrame(uint32_t frame)
		{
			ImGui::Begin("Inspector");

			if (scene_tree_.getSelected())
			{
				Enttity entity = *scene_tree_.getSelected();

				drawComponents(entity, frame);

				if (ImGui::Button("Add Component"))
					ImGui::OpenPopup("Add Component");

				if (ImGui::BeginPopup("Add Component"))
				{
					if (ImGui::MenuItem("Model Component"))
					{
						entity.addComponent<StaticModelComponent>(Mesh());
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
			}

			ImGui::End();
		}

	private:
		SceneTree& scene_tree_;

		void drawComponents(Enttity entity, uint32_t frame)
		{
			if (auto* comp = entity.tryGetComponent<TagComponent>())
			{
				char buf[256] = {0};
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
					bool edited = false;
					glm::vec3 rot_deg = comp->getRotationDegrees();
					if (drawVec3Control("Translation", comp->position))
						edited = true;
					if (drawVec3Control("Rotation", rot_deg))
					{
						comp->setDegrees(rot_deg);
						edited = true;
					}
					if (drawVec3Control("Scale", comp->scale, 1.0f))
						edited = true;

					if (edited)
						entity.markTransformAsEdited(frame);

					ImGui::TreePop();
				}
			}

			if (auto* comp = entity.tryGetComponent<StaticModelComponent>())
			{
				if (ImGui::TreeNodeEx("ModelComponent", ImGuiTreeNodeFlags_DefaultOpen))
				{
					//const Mesh* model = comp->getModel();
					//std::string text = model ? model->getPath() : "";
					//char buf[256];
					//memset(buf, 0, sizeof(buf));
					//strcpy_s(buf, sizeof(buf), text.c_str());
					//bool newPath = false;
					//ImGui::Text("Model: ");
					//ImGui::SameLine();
					//if (ImGui::InputText("##ModelPathLabel", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue))
					//{
					//	//comp->setNewModel(buf);
					//}
					//
					ImGui::TreePop();
				}
			}

			if (auto* comp = entity.tryGetComponent<AnimationComponent>())
			{
				if (ImGui::TreeNodeEx("AnimationComponent", ImGuiTreeNodeFlags_DefaultOpen))
				{
					for (unsigned i = 0; i < comp->animation_clips.size(); ++i)
					{
						ImGui::Text(comp->animation_clips[i].name.c_str());
					}

					ImGui::TreePop();
				}
			}
		}

		static bool drawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f,
		                            float columnWidth = 100.0f)
		{
			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];
			bool edited = false;

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
			{
				values.x = resetValue;
				edited = true;
			}
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
				edited = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
			ImGui::PushFont(boldFont);
			if (ImGui::Button("Y", buttonSize))
			{
				values.y = resetValue;
				edited = true;
			}
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
				edited = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
			ImGui::PushFont(boldFont);
			if (ImGui::Button("Z", buttonSize))
			{
				values.z = resetValue;
				edited = true;
			}
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			if (ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f"))
				edited = true;
			ImGui::PopItemWidth();

			ImGui::PopStyleVar();

			ImGui::Columns(1);

			ImGui::PopID();

			return edited;
		}
	};
}
