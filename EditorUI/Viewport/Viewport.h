#pragma once
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "Wrappers/Singletons/Renderer.h"
#include "ViewportRenderPass.h"
#include "ViewportSwapChain.h"
#include "EditorUI/Viewport/ViewportCamera.h"
#include "Main/Enttity.h"

namespace dmbrn
{
	class Viewport
	{
	public:
		const static inline ViewportRenderPass render_pass_;

		Viewport(Scene& scene, Enttity* selected, const std::string& name = "Viewport"):
			window_name_(name),
			size_(1280, 720),
			camera_(size_),
			swap_chain_({static_cast<unsigned>(size_.x), static_cast<unsigned>(size_.y)},
			            render_pass_),
			scene_(scene),
			selected_(selected)
		{
			for (int i = 0; i < swap_chain_.getFrameBuffers().size(); ++i)
			{
				const Texture& buf = swap_chain_.getColorBuffers()[i];
				images_.push_back(ImGui_ImplVulkan_AddTexture(*buf.getSampler(), *buf.getImageView(),
				                                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
			}
			last_used_focused = this;
		}

		void newImGuiFrame(float delta_t,uint32_t frame,uint32_t imageIndex)
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

			auto pos = ImGui::GetCursorPos();

			ImGui::Image(images_[imageIndex], size_);

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
					ImGuizmo::SetOrthographic(false);
					ImGuizmo::SetDrawlist();
					float windowWidth = (float)ImGui::GetWindowWidth();
					float windowHeight = (float)ImGui::GetWindowHeight();
					ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

					glm::mat4 cameraProj = camera_.camera_comp.getMatrix();
					glm::mat4 cameraView = camera_.getViewMat();

					cameraProj[1][1] *= -1;

					TransformComponent& t_c = selected_->getComponent<TransformComponent>();

					glm::mat4 local_trans = t_c.getMatrix();

					// TODO: parent trans

					if(ImGuizmo::Manipulate(glm::value_ptr(cameraView * local_trans), glm::value_ptr(cameraProj),
					                     current_operation, ImGuizmo::MODE::LOCAL,
					                     glm::value_ptr(local_trans)))
						selected_->markTransformAsEdited(frame);

					if (ImGuizmo::IsUsing())
					{
						ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(local_trans), glm::value_ptr(t_c.position),
						                                      glm::value_ptr(t_c.rotation), glm::value_ptr(t_c.scale));
						last_used_focused = this;
					}
				}
			}

			ImGui::End();
			ImGui::PopStyleVar();
		}

		void render(const LogicalDevice& device, const vk::raii::CommandBuffer& command_buffer,
		            uint32_t current_frame,
		            uint32_t imageIndex)
		{
			const Texture& color_buffer = swap_chain_.getColorBuffers()[imageIndex];

			color_buffer.transitionImageLayoutWithCommandBuffer(command_buffer, vk::ImageLayout::eShaderReadOnlyOptimal,
			                                                    vk::ImageLayout::eColorAttachmentOptimal);

			const std::array<vk::ClearValue, 2> clear_values
			{
				vk::ClearValue{vk::ClearColorValue{std::array<float, 4>{0.3f, 0.3f, 0.3f, 1.0f}}},
				vk::ClearValue{vk::ClearDepthStencilValue{1.0f, 0}}
			};

			const vk::RenderPassBeginInfo renderPassInfo
			{
				**render_pass_,
				*swap_chain_.getFrameBuffers()[imageIndex],
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

			//model drawing
			auto group = scene_.getModelsToDraw();

			for (auto entity : group)
			{
				auto [model,_] = group.get<ModelComponent, TransformComponent>(entity);

				model.getShader()->addToRenderQueue({&model.mesh, model.inGPU_transform_offset});
			}

			Renderer::un_lit_textured.draw(current_frame, command_buffer, ModelComponent::per_object_data_buffer_);
			Renderer::outlined_.draw(current_frame, command_buffer, ModelComponent::per_object_data_buffer_);

			command_buffer.endRenderPass();
		}

	private:
		std::string window_name_ = "Viewport";
		ImVec2 size_;
		ViewportCamera camera_;
		ViewportSwapChain swap_chain_;
		std::vector<VkDescriptorSet> images_;
		Scene& scene_;
		Enttity* selected_;
		static inline Viewport* last_used_focused = nullptr;
		static inline ImGuizmo::OPERATION current_operation = ImGuizmo::OPERATION::TRANSLATE;

		bool HandleWindowResize()
		{
			ImVec2 view = ImGui::GetContentRegionAvail();

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

			for (int i = 0; i < swap_chain_.getFrameBuffers().size(); ++i)
			{
				const Texture& buf = swap_chain_.getColorBuffers()[i];
				images_[i] = (ImGui_ImplVulkan_AddTexture(*buf.getSampler(), *buf.getImageView(),
				                                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
			}

			camera_.updateAspectRatio({size_.x, size_.y});
		}
	};
}
