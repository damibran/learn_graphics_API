#pragma once
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Wrappers/Singletons/Renderer.h"
#include "ViewportRenderPass.h"
#include "ViewportSwapChain.h"
#include "EditorUI/Viewport/ViewportCamera.h"

namespace dmbrn
{
	class Viewport
	{
	public:
		const static inline ViewportRenderPass render_pass_;

		Viewport(Scene& scene, const std::string& name = "Viewport") :
			window_name_(name),
			size_(1280, 720),
			camera_(size_),
			swap_chain_({static_cast<unsigned>(size_.x), static_cast<unsigned>(size_.y)},
			            render_pass_),
			scene_(scene)
		{
			for (int i = 0; i < swap_chain_.getFrameBuffers().size(); ++i)
			{
				const Texture& buf = swap_chain_.getColorBuffers()[i];
				images_.push_back(ImGui_ImplVulkan_AddTexture(*buf.getSampler(), *buf.getImageView(),
				                                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
			}
		}

		void newImGuiFrame(float delta_t, uint32_t imageIndex)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});

			ImGui::Begin(window_name_.c_str());

			if (HandleWindowResize() == false)
			{
				ImGui::End();
				return;
			}

			camera_.update(delta_t);

			ImGui::Image(images_[imageIndex], size_);

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
				auto [model,transform] = group.get<ModelComponent, TransformComponent>(entity);
				model.addToRenderQueue(transform.getMatrix());
			}

			Renderer::un_lit_textured.draw(current_frame, command_buffer);

			command_buffer.endRenderPass();
		}

	private:
		std::string window_name_ = "Viewport";
		ImVec2 size_;
		ViewportCamera camera_;
		ViewportSwapChain swap_chain_;
		std::vector<VkDescriptorSet> images_;
		Scene& scene_;

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
