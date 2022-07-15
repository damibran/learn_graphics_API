#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <optional>

#include "GLFWwindowWrapper.h"
#include "SurfaceWrapper.h"
#include "PhysicalDeviceWrapper.h"
#include "LogicalDeviceWrapper.h"

namespace dmbrn
{
	class SwapChain
	{
	public:
		SwapChain(const PhysicalDeviceWrapper& physical_device,
			const LogicalDeviceWrapper& device,
			const SurfaceWrapper& surface,
			const GLFWwindowWrapper& window)
		{
			PhysicalDeviceWrapper::SwapChainSupportDetails swapChainSupport = physical_device.getSwapChainSupportDetails();

			vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
			vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
			vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

			uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
			if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
				imageCount = swapChainSupport.capabilities.maxImageCount;
			}

			vk::SwapchainCreateInfoKHR createInfo{};
			createInfo.surface = **surface;

			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

			PhysicalDeviceWrapper::QueueFamilyIndices indices = physical_device.getQueueFamilyIndices();
			uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

			if (indices.graphicsFamily != indices.presentFamily) {
				createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else {
				createInfo.imageSharingMode = vk::SharingMode::eExclusive;
			}

			createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
			createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
			createInfo.presentMode = presentMode;
			createInfo.clipped = VK_TRUE;

			swap_chain_ = std::make_unique<vk::raii::SwapchainKHR>(device->createSwapchainKHR(createInfo));

			images_ = swap_chain_->getImages();

			swap_chain_image_format_ = surfaceFormat.format;
			swap_chain_extent_ = extent;

			createImageViews(device);
		}
	private:
		std::unique_ptr<vk::raii::SwapchainKHR> swap_chain_;
		std::vector<VkImage> images_;
		std::vector<vk::raii::ImageView> image_views_;
		vk::Format swap_chain_image_format_;
		vk::Extent2D swap_chain_extent_;

		vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
		{
			for (const auto& availableFormat : availableFormats)
			{
				if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
				{
					return availableFormat;
				}
			}

			return availableFormats[0];
		}

		void createImageViews(const LogicalDeviceWrapper& device)
		{
			for (size_t i = 0; i < images_.size(); i++)
			{
				image_views_.push_back(createImageView(device, images_[i], swap_chain_image_format_));
			}
		}

		vk::raii::ImageView createImageView(const LogicalDeviceWrapper& device, VkImage image, vk::Format format)
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

		vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
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

		vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const GLFWwindowWrapper& window)
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