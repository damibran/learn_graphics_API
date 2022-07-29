#pragma once
#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "CommandBuffers.h"
#include "ImGUIRenderPass.h"
#include "Singletons.h"


namespace dmbrn
{
	struct EditorFrame
	{
		vk::raii::CommandBuffer command_buffer;
		vk::raii::Semaphore image_available_semaphore;
		vk::raii::Semaphore render_finished_semaphore;
		vk::raii::Fence in_flight_fence;
		vk::raii::ImageView image_view;
		vk::raii::Framebuffer frame_buffer;

		EditorFrame(const Singletons& singletons, const ImGUIRenderPass& render_pass,vk::Format format,const vk::Extent2D& extent,
		            const VkImage& image) :
			command_buffer(std::move(createCommandBuffer(singletons.device, singletons.command_pool)[0])),
			image_available_semaphore(singletons.device->createSemaphore({})),
			render_finished_semaphore(singletons.device->createSemaphore({})),
			in_flight_fence(singletons.device->createFence({vk::FenceCreateFlagBits::eSignaled})),
			image_view(createImageView(singletons.device, image, format)),
			frame_buffer(createFrameBuffers(singletons.device, render_pass, extent))
		{
		}

		void resize(const Singletons& singletons, const ImGUIRenderPass& render_pass,vk::Format format,const vk::Extent2D& extent,
		            const VkImage& image)
		{
			image_view = createImageView(singletons.device, image, format);
			frame_buffer = createFrameBuffers(singletons.device, render_pass, extent);
		}

	private:
		std::vector<vk::raii::CommandBuffer> createCommandBuffer(const LogicalDevice& device,
		                                                            const CommandPool& command_pool)
		{
			const vk::CommandBufferAllocateInfo allocInfo
			{
				**command_pool, vk::CommandBufferLevel::ePrimary,static_cast<uint32_t>(1)
			};

			return device->allocateCommandBuffers(allocInfo);
		}

		[[nodiscard]] vk::raii::ImageView createImageView(const LogicalDevice& device, const VkImage& image,
		                                                  vk::Format format)
		{
			const vk::ImageViewCreateInfo viewInfo
			{
				{}, image, vk::ImageViewType::e2D,
				format, {},
				vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
			};

			return device->createImageView(viewInfo);
		}

		[[nodiscard]] vk::raii::Framebuffer createFrameBuffers(
			const LogicalDevice& device, const ImGUIRenderPass& render_pass, const vk::Extent2D& extent)
		{
			const vk::ImageView attachments[] = {
				*image_view,
			};

			const vk::FramebufferCreateInfo framebufferInfo
			{
				{}, **render_pass,
				attachments, extent.width, extent.height, 1
			};


			return device->createFramebuffer(framebufferInfo);
		}
	};
	;
}
