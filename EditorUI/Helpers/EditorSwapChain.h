#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <optional>

#include "Wrappers/Singletons/GLFWwindowWrapper.h"
#include "Wrappers/Singletons/Surface.h"
#include "Wrappers/Singletons/PhysicalDevice.h"
#include "Wrappers/Singletons/LogicalDevice.h"
#include "EditorRenderPass.h"
#include "Utils/UtilsFunctions.h"
#include "EditorFrame.h"

namespace dmbrn
{
	/**
	 * \brief represents editor UI swap chain
	 */
	class EditorSwapChain
	{
	public:
		EditorSwapChain(const EditorRenderPass& render_pass):
			swap_chain_(createSwapChain(Singletons::physical_device, Singletons::device, Singletons::surface, Singletons::window))
		{
			for (auto image : swap_chain_.getImages())
			{
				editor_frames_.emplace_back(render_pass, image_format_, extent_, image);
			}
		}

		void recreate(const EditorRenderPass& render_pass)
		{
			int width = 0, height = 0;
			const auto rec_size = Singletons::window.getFrameBufferSize();
			width = rec_size.first;
			height = rec_size.second;
			while (width == 0 || height == 0)
			{
				const auto rec_size = Singletons::window.getFrameBufferSize();
				width = rec_size.first;
				height = rec_size.second;
				glfwWaitEvents();
			}

			Singletons::device->waitIdle();

			swap_chain_ = createSwapChain(Singletons::physical_device, Singletons::device, Singletons::surface,
			                              Singletons::window);
			auto frame = editor_frames_.begin();
			for (auto image : swap_chain_.getImages())
			{
				*frame = EditorFrame{render_pass, image_format_, extent_, image};
				++frame;
			}
		}

		const vk::raii::SwapchainKHR& operator*() const
		{
			return swap_chain_;
		}

		const vk::raii::SwapchainKHR* operator->() const
		{
			return &swap_chain_;
		}

		const EditorFrame& getFrame(uint32_t i)
		{
			return editor_frames_[i];
		}

		const vk::Format& getFormat() const
		{
			return image_format_;
		}

		const vk::Extent2D& getExtent() const
		{
			return extent_;
		}

	private:
		vk::Format image_format_ = vk::Format::eR8G8B8A8Unorm;
		vk::Extent2D extent_{1280, 720};
		vk::raii::SwapchainKHR swap_chain_;
		std::vector<EditorFrame> editor_frames_;

		[[nodiscard]] vk::raii::SwapchainKHR createSwapChain(const PhysicalDevice& physical_device,
		                                                     const LogicalDevice& device,
		                                                     const Surface& surface,
		                                                     const GLFWwindowWrapper& window)
		{
			const auto capabilities = PhysicalDevice::querySurfaceCapabilities(*physical_device, surface);

			const vk::SurfaceFormatKHR surfaceFormat = utils::chooseSwapSurfaceFormat(
				PhysicalDevice::querySurfaceFormats(*physical_device, surface));
			const vk::PresentModeKHR presentMode = chooseSwapPresentMode(
				PhysicalDevice::querySurfacePresentModes(*physical_device, surface));
			const vk::Extent2D extent = utils::chooseSwapExtent(capabilities, window);

			const uint32_t imageCount = utils::capabilitiesGetImageCount(physical_device, surface);

			vk::SwapchainCreateInfoKHR createInfo{}; // very easy to miss something... may be redo
			createInfo.surface = **surface;
			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

			const PhysicalDevice::QueueFamilyIndices indices = physical_device.getQueueFamilyIndices();
			const uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

			if (indices.graphicsFamily != indices.presentFamily)
			{
				createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else
			{
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

		static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
		{
			//for (const auto& availablePresentMode : availablePresentModes)
			//{
			//	if (availablePresentMode == vk::PresentModeKHR::eMailbox)
			//	{
			//		return availablePresentMode;
			//	}
			//}

			return vk::PresentModeKHR::eFifo;
		}
	};
}
