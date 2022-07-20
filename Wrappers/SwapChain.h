#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <optional>

#include "GLFWwindowWrapper.h"
#include "Surface.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "RenderPass.h"
#include "../Utils/UtilsFunctions.h"

namespace dmbrn
{
	class SwapChain
	{
	public:
		SwapChain(const PhysicalDevice& physical_device,
			const LogicalDevice& device,
			const Surface& surface, const GLFWwindowWrapper& window, const RenderPass& render_pass)
		{
			createSwapChain(physical_device, device, surface, window);
			createImageViews(device);
			createFrameBuffers(device, render_pass);
		}

		void recreate(const PhysicalDevice& physical_device,
			const LogicalDevice& device,
			const Surface& surface,
			const GLFWwindowWrapper& window, const RenderPass& render_pass)
		{
			int width = 0, height = 0;
			auto rec_size = window.getFrameBufferSize();
			width = rec_size.first;
			height = rec_size.second;
			while (width == 0 || height == 0) {
				auto rec_size = window.getFrameBufferSize();
				width = rec_size.first;
				height = rec_size.second;
				glfwWaitEvents();
			}

			device->waitIdle();

			framebuffers_.clear();
			image_views_.clear();

			createSwapChain(physical_device, device, surface, window);
			createImageViews(device);
			createFrameBuffers(device, render_pass);
		}

		const vk::raii::SwapchainKHR& operator*()const
		{
			return *swap_chain_;
		}

		const vk::raii::SwapchainKHR* operator->()const
		{
			return swap_chain_.get();
		}

		const vk::Extent2D& getExtent()const
		{
			return extent_;
		}

		const std::vector<vk::raii::ImageView>& getImageViews()const
		{
			return image_views_;
		}

		const std::vector<vk::raii::Framebuffer>& getFrameBuffers()const
		{
			return framebuffers_;
		}

	private:
		std::unique_ptr<vk::raii::SwapchainKHR> swap_chain_;
		std::vector<VkImage> images_;
		std::vector<vk::raii::ImageView> image_views_;
		std::vector<vk::raii::Framebuffer> framebuffers_;
		vk::Format image_format_;
		vk::Extent2D extent_;

		void createSwapChain(const PhysicalDevice& physical_device,
			const LogicalDevice& device,
			const Surface& surface,
			const GLFWwindowWrapper& window)
		{

			auto capabilities = PhysicalDevice::querySurfaceCapabilities(*physical_device, surface);

			vk::SurfaceFormatKHR surfaceFormat = utils::chooseSwapSurfaceFormat(PhysicalDevice::querySurfaceFormats(*physical_device, surface));
			vk::PresentModeKHR presentMode = chooseSwapPresentMode(PhysicalDevice::querySurfacePresentModes(*physical_device, surface));
			vk::Extent2D extent = chooseSwapExtent(capabilities, window);

			uint32_t imageCount = capabilities.minImageCount + 1;
			if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
				imageCount = capabilities.maxImageCount;
			}

			vk::SwapchainCreateInfoKHR createInfo{};
			createInfo.surface = **surface;

			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

			PhysicalDevice::QueueFamilyIndices indices = physical_device.getQueueFamilyIndices();
			uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

			if (indices.graphicsFamily != indices.presentFamily) {
				createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else {
				createInfo.imageSharingMode = vk::SharingMode::eExclusive;
			}

			createInfo.preTransform = capabilities.currentTransform;
			createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
			createInfo.presentMode = presentMode;
			createInfo.clipped = VK_TRUE;
			if (swap_chain_.get())
				createInfo.oldSwapchain = **swap_chain_;

			swap_chain_ = std::make_unique<vk::raii::SwapchainKHR>(device->createSwapchainKHR(createInfo));

			images_ = swap_chain_->getImages();

			image_format_ = surfaceFormat.format;
			extent_ = extent;
		}

		void createImageViews(const LogicalDevice& device)
		{
			for (size_t i = 0; i < images_.size(); i++)
			{
				image_views_.push_back(createImageView(device, images_[i], image_format_));
			}
		}

		vk::raii::ImageView createImageView(const LogicalDevice& device, VkImage image, vk::Format format)
		{
			vk::ImageViewCreateInfo viewInfo{};
			viewInfo.image = image;
			viewInfo.viewType = vk::ImageViewType::e2D;
			viewInfo.format = format;
			viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			return device->createImageView(viewInfo);
		}

		void createFrameBuffers(const LogicalDevice& device, const RenderPass& render_pass)
		{
			for (size_t i = 0; i < image_views_.size(); i++)
			{
				vk::ImageView attachments[] = {
					*image_views_[i]
				};

				vk::FramebufferCreateInfo framebufferInfo{};
				framebufferInfo.renderPass = **render_pass;
				framebufferInfo.attachmentCount = 1;
				framebufferInfo.pAttachments = attachments;
				framebufferInfo.width = extent_.width;
				framebufferInfo.height = extent_.height;
				framebufferInfo.layers = 1;

				framebuffers_.push_back(device->createFramebuffer(framebufferInfo));
			}
		}

		static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
		{
			for (const auto& availablePresentMode : availablePresentModes)
			{
				if (availablePresentMode == vk::PresentModeKHR::eMailbox)
				{
					return availablePresentMode;
				}
			}

			return vk::PresentModeKHR::eFifo;
		}

		static vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const GLFWwindowWrapper& window)
		{
			if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			{
				return capabilities.currentExtent;
			}
			else
			{
				auto rect = window.getFrameBufferSize();

				vk::Extent2D actualExtent = {
					static_cast<uint32_t>(rect.first),
					static_cast<uint32_t>(rect.second)
				};

				actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

				return actualExtent;
			}
		}
	};
}