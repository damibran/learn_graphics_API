#pragma once
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "Main/Enttity.h"
#include "ViewportRenderPass.h"
#include "ViewportSwapChain.h"
#include "EditorUI/Viewport/ViewportCamera.h"
#include "Utils/Transform.h"
#include "EditorUI/Sequencer/Sequencer.h"

namespace dmbrn
{
	/**
	 * \brief represents viewport UI window
	 */
	class Viewport
	{
	public:
		const static inline ViewportRenderPass render_pass_;

		Viewport(Scene& scene, Sequencer& sequencer, Enttity* selected, const std::string& name = "Viewport"):
			window_name_(name),
			size_(1280, 720),
			camera_(size_),
			swap_chain_({static_cast<unsigned>(size_.x), static_cast<unsigned>(size_.y)},
			            render_pass_),
			scene_(scene),
			selected_(selected),
			sequencer_(sequencer)
		{
			last_used_focused = this;
		}

		void newImGuiFrame(double delta_t, uint32_t frame, uint32_t imageIndex)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});

			ImGui::Begin(window_name_.c_str());

			if (HandleWindowResize() == false)
			{
				ImGui::End();
				return;
			}

			if (!ImGuizmo::IsUsing())
			{
				camera_.update(delta_t);
			}

			if (ImGui::IsWindowFocused())
				last_used_focused = this;

			const auto pos = ImGui::GetCursorPos();

			ImGui::Image(swap_chain_.getImGuiImageWithIndex(imageIndex), size_);

			ImGui::SetItemAllowOverlap();

			ImGui::SetCursorPos(ImVec2(pos.x + 5, pos.y));

			if (ImGui::Selectable("T", current_operation == ImGuizmo::OPERATION::TRANSLATE, 0, {10, 0}) ||
				ImGui::IsWindowFocused() && !ImGui::IsMouseDown(camera_.pan_button) && ImGui::IsKeyPressed(ImGuiKey_W))
				current_operation = ImGuizmo::OPERATION::TRANSLATE;
			ImGui::SameLine();
			if (ImGui::Selectable("R", current_operation == ImGuizmo::OPERATION::ROTATE, 0, {10, 0}) ||
				ImGui::IsWindowFocused() && !ImGui::IsMouseDown(camera_.pan_button) && ImGui::IsKeyPressed(ImGuiKey_E))
				current_operation = ImGuizmo::OPERATION::ROTATE;
			ImGui::SameLine();
			if (ImGui::Selectable("S", current_operation == ImGuizmo::OPERATION::SCALE, 0, {10, 0}) ||
				ImGui::IsWindowFocused() && !ImGui::IsMouseDown(camera_.pan_button) && ImGui::IsKeyPressed(ImGuiKey_R))
				current_operation = ImGuizmo::OPERATION::SCALE;

			if (this == last_used_focused)
			{
				if (*selected_)
				{
					float size_clip_space = 0.1f;
					ImGuizmo::SetOrthographic(false);
					ImGuizmo::SetDrawlist();
					const float windowWidth = ImGui::GetWindowWidth();
					const float windowHeight = ImGui::GetWindowHeight();
					ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

					glm::mat4 cameraProj = camera_.camera_comp.getMatrix();
					const glm::mat4 cameraView = camera_.getViewMat();

					cameraProj[1][1] *= -1;

					TransformComponent& t_c = selected_->getComponent<TransformComponent>();

					const glm::mat4& parent_trans = selected_->getComponent<RelationshipComponent>().parent.getComponent
						<TransformComponent>().globalTransformMatrix;

					glm::mat4 local_trans = t_c.getMatrix();

					const glm::vec3 parent_glob_scale = getScale(parent_trans);

					const float avg_scale = (parent_glob_scale.x + parent_glob_scale.y + parent_glob_scale.z) / 3;

					size_clip_space = size_clip_space / avg_scale;

					ImGuizmo::SetGizmoSizeClipSpace(size_clip_space);

					if (ImGuizmo::Manipulate(glm::value_ptr(cameraView * parent_trans), glm::value_ptr(cameraProj),
					                         current_operation, ImGuizmo::MODE::LOCAL,
					                         glm::value_ptr(local_trans)))

						if (ImGuizmo::IsUsing())
						{
							Transform new_trans;
							ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(local_trans),
							                                      glm::value_ptr(new_trans.position),
							                                      glm::value_ptr(new_trans.rotation),
							                                      glm::value_ptr(new_trans.scale));
							if (sequencer_.hasRecordingEnts())
							{
								Enttity rec_parent = selected_->findRecordingAnimationCompParent();
								if (rec_parent)
								{
									if (new_trans.position != t_c.position)
										sequencer_.processKey<AnimationChannels::PosKeyTag>(selected_,rec_parent,new_trans.position);
									if (new_trans.rotation != t_c.getRotationDegrees())
										sequencer_.processKey<AnimationChannels::RotKeyTag>(selected_,rec_parent,glm::quat{glm::radians(new_trans.rotation)});
									if (new_trans.scale != t_c.scale)
										sequencer_.processKey<AnimationChannels::ScaleKeyTag>(selected_,rec_parent,new_trans.scale);
								}
							}

							t_c.position = new_trans.position;
							t_c.setDegrees(new_trans.rotation);
							t_c.scale = new_trans.scale;

							selected_->markTransformAsEdited(frame);

							last_used_focused = this;
						}
				}
			}

			ImGui::End();
			ImGui::PopStyleVar();
		}

		void render(const vk::raii::CommandBuffer& command_buffer,
		            uint32_t current_frame,
		            uint32_t imageIndex)
		{
			const std::array<vk::ClearValue, 2> clear_values
			{
				vk::ClearValue{vk::ClearColorValue{std::array<float, 4>{0.3f, 0.3f, 0.3f, 1.0f}}},
				vk::ClearValue{vk::ClearDepthStencilValue{1.0f, 0}}
			};

			const vk::RenderPassBeginInfo renderPassInfo
			{
				**render_pass_,
				*swap_chain_.getFrameBufferWithIndex(imageIndex),
				vk::Rect2D{vk::Offset2D{0, 0}, swap_chain_.getExtent()},
				clear_values
			};

			command_buffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

			const vk::Viewport viewport
			{
				0.0f, 0.0f,
				static_cast<float>(swap_chain_.getExtent().width),
				static_cast<float>(swap_chain_.getExtent().height),
				0.0f, 1.0f
			};
			command_buffer.setViewport(0, viewport);

			const vk::Rect2D scissor
			{
				vk::Offset2D{0, 0},
				swap_chain_.getExtent()
			};
			command_buffer.setScissor(0, scissor);

			Renderer::newView(current_frame, camera_, command_buffer);

			// static model drawing
			auto static_view = scene_.getModelsToDraw();
			for (auto entity : static_view)
			{
				StaticModelComponent& model = static_view.get<StaticModelComponent>(entity);
				model.getShader()->addToRenderQueue({&model.mesh, model.inGPU_transform_offset});
			}

			// skeletal model drawing
			auto skeletal_group = scene_.getSkeletalModelsToDraw();
			for (auto entity : skeletal_group)
			{
				SkeletalModelComponent& skeletal_model = skeletal_group.get<
					SkeletalModelComponent>(entity);
				skeletal_model.getShader()->addToRenderQueue({
					&skeletal_model.mesh,
					skeletal_model.skeleton_ent.getComponent<SkeletonComponent>().in_GPU_mtxs_offset
				});
			}

			Renderer::un_lit_textured.draw(current_frame, command_buffer);
			Renderer::outlined_.draw(current_frame, command_buffer);

			command_buffer.endRenderPass();
		}

	private:
		std::string window_name_ = "Viewport";
		ImVec2 size_;
		ViewportCamera camera_;
		ViewportSwapChain swap_chain_;
		Scene& scene_;
		Enttity* selected_;
		Sequencer& sequencer_;
		static inline Viewport* last_used_focused = nullptr;
		static inline ImGuizmo::OPERATION current_operation = ImGuizmo::OPERATION::TRANSLATE;

		bool HandleWindowResize()
		{
			const ImVec2 view = ImGui::GetContentRegionAvail();

			if (view.x != size_.x || view.y != size_.y)
			{
				if (view.x == 0 || view.y == 0 || ImGui::IsWindowCollapsed())
				{
					// The window is too small or collapsed.
					return false;
				}

				size_.x = view.x;
				size_.y = view.y;

				resize();

				// The window state has been successfully changed.
				return true;
			}

			// The window state has not changed.
			return true;
		}

		void resize()
		{
			swap_chain_.recreate({static_cast<unsigned>(size_.x), static_cast<unsigned>(size_.y)},
			                     render_pass_);

			camera_.updateAspectRatio({size_.x, size_.y});
		}
	};
}
