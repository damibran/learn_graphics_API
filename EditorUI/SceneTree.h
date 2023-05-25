#pragma once
#include "ECS/Scene.h"

namespace dmbrn
{
	/**
	 * \brief represents scene tree UI window
	 */
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

			if (ImGui::BeginTable("SceneTreeTable", 1,ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_BordersInnerH))
			{
				ImGui::TableNextColumn();

				if (ImGui::Button("Add new from file"))
				{
					ImGui::OpenPopup("Import_Model_Modal");
					import_model_modal_vars_.modal_opened = true;
				}

				showImportWindow();

				ImGui::TableNextColumn();

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

				ImGui::EndTable();
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

		struct ImportModelModalVars
		{
			bool modal_opened = false;
			std::string model_path;
			bool with_bones = false;
			bool with_anim = false;
		} import_model_modal_vars_;

		void recursivelyDraw(Enttity enttity)
		{
			const TagComponent& tag = enttity.getComponent<TagComponent>();
			const RelationshipComponent& relation_comp = enttity.getComponent<RelationshipComponent>();

			const ImGuiTreeNodeFlags flags =
				(selected_ == enttity ? ImGuiTreeNodeFlags_Selected : 0) |
				(!relation_comp.first ? ImGuiTreeNodeFlags_Leaf : 0) |
				ImGuiTreeNodeFlags_OpenOnArrow;

			const bool opened = ImGui::TreeNodeEx(
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

		void showImportWindow()
		{
			char buf[256] = {0};

			if (ImGui::BeginPopupModal("Import_Model_Modal", &import_model_modal_vars_.modal_opened,
			                           ImGuiWindowFlags_AlwaysAutoResize))
			{
				strcpy_s(buf, sizeof(buf), import_model_modal_vars_.model_path.c_str());

				if (ImGui::InputText("Model path", buf, sizeof(buf)))
				{
					import_model_modal_vars_.model_path = std::string(buf);
				}

				ImGui::Checkbox("Import with animations", &import_model_modal_vars_.with_anim);
				ImGui::Checkbox("Import with bones", &import_model_modal_vars_.with_bones);

				if (ImGui::Button("Import"))
				{
					scene_.importModel(import_model_modal_vars_.model_path, import_model_modal_vars_.with_bones,
					                   import_model_modal_vars_.with_anim);
					ImGui::CloseCurrentPopup();
					import_model_modal_vars_.modal_opened = false;
				}

				ImGui::EndPopup();
			}
		}

		void deselect(Enttity enttity)
		{
			if (StaticModelComponent* model_comp = enttity.tryGetComponent<StaticModelComponent>())
			{
				model_comp->shader_ = &Renderer::un_lit_textured;
			}

			if(SkeletalModelComponent* skeletal_model_comp = enttity.tryGetComponent<SkeletalModelComponent>())
			{
				skeletal_model_comp->shader_ = &Renderer::un_lit_textured;
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
			if (StaticModelComponent* model_comp = enttity.tryGetComponent<StaticModelComponent>())
			{
				model_comp->shader_ = &Renderer::outlined_;
			}

			if(SkeletalModelComponent* skeletal_model_comp = enttity.tryGetComponent<SkeletalModelComponent>())
			{
				skeletal_model_comp->shader_ = &Renderer::outlined_;
			}

			Enttity cur_child = enttity.getComponent<RelationshipComponent>().first;
			while (cur_child)
			{
				select(cur_child);

				cur_child = cur_child.getComponent<RelationshipComponent>().next;
			}
		}
	};
}
