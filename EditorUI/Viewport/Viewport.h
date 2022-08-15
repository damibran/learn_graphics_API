#pragma once
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Wrappers/UniformBuffers.h"

#include "ViewportRenderPass.h"
#include "ViewportSwapChain.h"

namespace dmbrn
{
	class Viewport
	{
	public:
		Viewport(const Singletons& singletons) :
			size_(getWindowSize()),
			render_pass_(singletons.surface, singletons.physical_device, singletons.device),
			swap_chain_({static_cast<unsigned>(size_.x), static_cast<unsigned>(size_.y)}, singletons,
			            render_pass_),
		scene_(singletons,render_pass_)
		{
			for (int i = 0; i < swap_chain_.getFrameBuffers().size(); ++i)
			{
				const Texture& buf = swap_chain_.getColorBuffers()[i];
				images_.push_back(ImGui_ImplVulkan_AddTexture(*buf.getSampler(), *buf.getImageView(),
				                                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
			}
		}

		void newImGuiFrame(const Singletons& singletons, uint32_t imageIndex)
		{
			ImGui::Begin("Viewport");

			if (HandleWindowResize(singletons) == false)
			{
				ImGui::End();
				return;
			}

			ImGui::Image(images_[imageIndex], size_);

			ImGui::End();
		}

		void render(const LogicalDevice& device, const vk::raii::CommandBuffer& command_buffer, float delta_time,uint32_t current_frame,
		            uint32_t imageIndex)
		{
			const Texture& color_buffer = swap_chain_.getColorBuffers()[imageIndex];

			//updateUniformBuffer(current_frame, delta_time);

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

			//command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **graphics_pipeline_);

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

			//model drawing
			scene_.draw(current_frame, device,command_buffer);
			//model_.Draw(current_frame, device, graphics_pipeline_, command_buffer, descriptor_sets_);

			command_buffer.endRenderPass();
		}

	private:
		ImVec2 size_;
		ViewportRenderPass render_pass_;
		ViewportSwapChain swap_chain_;
		std::vector<VkDescriptorSet> images_;
		Scene scene_;

		bool HandleWindowResize(const Singletons& singletons)
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

				resize(singletons);

				// The window state has been successfully changed.
				return true;
			}

			// The window state has not changed.
			return true;
		}

		void resize(const Singletons& singletons)
		{
			swap_chain_.recreate({static_cast<unsigned>(size_.x), static_cast<unsigned>(size_.y)},
			                     singletons,
			                     render_pass_);

			for (int i = 0; i < swap_chain_.getFrameBuffers().size(); ++i)
			{
				const Texture& buf = swap_chain_.getColorBuffers()[i];
				images_[i] = (ImGui_ImplVulkan_AddTexture(*buf.getSampler(), *buf.getImageView(),
				                                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
			}
		}

		/*void updateUniformBuffer(uint32_t currentImage, float delta_t)
		{
			const float speed = 90;

			static float objAngle = 0;

			objAngle += delta_t * glm::radians(speed);

			UniformBuffers::UniformBufferObject ubo{};
			ubo.model = rotate(glm::mat4(1.0f), objAngle, glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
			                  glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.proj = glm::perspective(glm::radians(45.0f),
			                            swap_chain_.getExtent().width / static_cast<float>(swap_chain_.getExtent().
				                            height), 0.1f,
			                            10.0f);
			ubo.proj[1][1] *= -1;

			void* data = uniform_buffers_.getUBMemory(currentImage).mapMemory(0, sizeof(ubo));
			memcpy(data, &ubo, sizeof(ubo));
			uniform_buffers_.getUBMemory(currentImage).unmapMemory();
		}*/

		ImVec2 getWindowSize()
		{
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Viewport");

			ImVec2 res = ImGui::GetContentRegionAvail();

			ImGui::End();
			ImGui::EndFrame();

			ImGui::UpdatePlatformWindows();

			return res;
		}
	};
}
