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
#include "Depthbuffer.h"

namespace dmbrn
{
	class SwapChain
	{
	public:
		SwapChain(const PhysicalDevice& physical_device,
			const LogicalDevice& device, const Surface& surface,
			const GLFWwindowWrapper& window, const RenderPass& render_pass):
			swap_chain_(createSwapChain(physical_device, device, surface, window)),
			image_views_(createImageViews(device)),
			depth_buffer_(surface,window,physical_device,device),
			framebuffers_(createFrameBuffers(device, render_pass)),
			image_format_(image_format_),
			extent_(extent_)
		{
		}

		void recreate(const PhysicalDevice& physical_device,
			const LogicalDevice& device, const Surface& surface,
			const GLFWwindowWrapper& window, const RenderPass& render_pass)
		{
			int width = 0, height = 0;
			const auto rec_size = window.getFrameBufferSize();
			width = rec_size.first;
			height = rec_size.second;
			while (width == 0 || height == 0)
			{
				auto rec_size = window.getFrameBufferSize();
				width = rec_size.first;
				height = rec_size.second;
				glfwWaitEvents();
			}

			device->waitIdle();

			framebuffers_.clear();
			image_views_.clear();

			swap_chain_ = createSwapChain(physical_device, device, surface, window);
			image_views_ = createImageViews(device);
			depth_buffer_ = DepthBuffer(surface,window,physical_device,device);
			framebuffers_ = createFrameBuffers(device, render_pass);
		}

		const vk::raii::SwapchainKHR& operator*()const
		{
			return swap_chain_;
		}

		const vk::raii::SwapchainKHR* operator->()const
		{
			return &swap_chain_;
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
		vk::raii::SwapchainKHR swap_chain_;
		std::vector<vk::raii::ImageView> image_views_;
		DepthBuffer depth_buffer_;
		std::vector<vk::raii::Framebuffer> framebuffers_;
		vk::Format image_format_;
		vk::Extent2D extent_;

		[[nodiscard]] vk::raii::SwapchainKHR createSwapChain(const PhysicalDevice& physical_device,
			const LogicalDevice& device,
			const Surface& surface,
			const GLFWwindowWrapper& window)
		{
			const auto capabilities = PhysicalDevice::querySurfaceCapabilities(*physical_device, surface);

			const vk::SurfaceFormatKHR surfaceFormat = utils::chooseSwapSurfaceFormat(PhysicalDevice::querySurfaceFormats(*physical_device, surface));
			const vk::PresentModeKHR presentMode = chooseSwapPresentMode(PhysicalDevice::querySurfacePresentModes(*physical_device, surface));
			const vk::Extent2D extent = utils::chooseSwapExtent(capabilities, window);

			uint32_t imageCount = capabilities.minImageCount + 1;
			if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
				imageCount = capabilities.maxImageCount;
			}

			vk::SwapchainCreateInfoKHR createInfo{}; // very easy to miss something... may be redo
			createInfo.surface = **surface;
			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

			const PhysicalDevice::QueueFamilyIndices indices = physical_device.getQueueFamilyIndices();
			const uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

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
			createInfo.oldSwapchain = *swap_chain_;

			vk::raii::SwapchainKHR result = device->createSwapchainKHR(createInfo);

			image_format_ = surfaceFormat.format;
			extent_ = extent;

			return result;
		}

		[[nodiscard]] std::vector<vk::raii::ImageView> createImageViews(const LogicalDevice& device) const
		{
			std::vector<vk::raii::ImageView> result;
			std::vector<VkImage> images = swap_chain_.getImages();
			for (size_t i = 0; i < images.size(); i++)
			{
				result.push_back(createImageView(device, images[i], image_format_));
			}

			return result;
		}

		static vk::raii::ImageView createImageView(const LogicalDevice& device, VkImage image, vk::Format format)
		{
			const vk::ImageViewCreateInfo viewInfo
			{
				{},image, vk::ImageViewType::e2D,
				format,{},
				vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor,0,1,0,1}
			};

			return device->createImageView(viewInfo);
		}

		[[nodiscard]] std::vector<vk::raii::Framebuffer> createFrameBuffers(const LogicalDevice& device, const RenderPass& render_pass)
		{
			std::vector<vk::raii::Framebuffer> result;

			for (size_t i = 0; i < image_views_.size(); i++)
			{
				const vk::ImageView attachments[] = {
					*image_views_[i],
					**depth_buffer_
				};

				const vk::FramebufferCreateInfo framebufferInfo
				{
					{},**render_pass,
					attachments,extent_.width,extent_.height,1
				};

				result.push_back(device->createFramebuffer(framebufferInfo));
			}

			return result;
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
	};
}