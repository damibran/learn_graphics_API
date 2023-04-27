#pragma once

#include "SceneTree.h"

namespace dmbrn
{
	class Inspector
	{
	public:
		Inspector(Scene& scene, SceneTree& scene_tree):
		scene_(scene),
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
		Scene& scene_;
		SceneTree& scene_tree_;
		std::string file_path;

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
					char buf[256] = {0};
					ImGui::Spacing();
					ImGui::Text("Animation clips:");
					if (ImGui::BeginTable("Animation_clips_table", 1,
					                      ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings |
					                      ImGuiTableFlags_Borders))
					{
						int i=1;
						for (decltype(comp->animation_clips)::iterator clip_it = comp->animation_clips.begin();clip_it!=comp->animation_clips.end();++clip_it,++i)
						{
							ImGui::TableNextColumn();

							strcpy_s(buf, sizeof(buf), clip_it->name.data());

							std::string label = "Clip "+std::to_string(i);

							if(ImGui::InputText(label.c_str(),buf,sizeof(buf),ImGuiInputTextFlags_EnterReturnsTrue))
							{
								comp->updateClipName(std::make_move_iterator(clip_it),std::string(buf));
								break;
							}

							if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID|ImGuiDragDropFlags_AcceptBeforeDelivery))
							{
								auto payload_data = std::make_pair(entity,&*clip_it);
								ImGui::SetDragDropPayload("Animation_clip_DnD",&payload_data,sizeof(payload_data));

								ImGui::Text(clip_it->name.c_str());
								ImGui::EndDragDropSource();
							}
						}
						ImGui::EndTable();
					}
					ImGui::Spacing();
					// TODO filedialog
					if (ImGui::Button("Add animation from file"))
						ImGui::OpenPopup("Import_Animation_Modal");

					if (ImGui::BeginPopupModal("Import_Animation_Modal",NULL,ImGuiWindowFlags_AlwaysAutoResize))
					{
						strcpy_s(buf, sizeof(buf), file_path.c_str());

						if (ImGui::InputText("Model path", buf, sizeof(buf)))
						{
							file_path = std::string(buf);
						}

						if (ImGui::Button("Import"))
						{
							scene_.importAnimationTo(entity,file_path);
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
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
