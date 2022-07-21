#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>

namespace dmbrn
{
	namespace utils
	{
		vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
		{
			for (const auto& availableFormat : availableFormats) {
				if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
					return availableFormat;
				}
			}

			return availableFormats[0];
		}

		vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const GLFWwindowWrapper& window)
		{
			if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			{
				return capabilities.currentExtent;
			}
			else
			{
				const auto rect = window.getFrameBufferSize();

				vk::Extent2D actualExtent = {
					static_cast<uint32_t>(rect.first),
					static_cast<uint32_t>(rect.second)
				};

				actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

				return actualExtent;
			}
		}

		vk::Format findSupportedFormat(const PhysicalDevice& physical_device, const std::vector<vk::Format>& candidates,
			vk::ImageTiling tiling, vk::FormatFeatureFlags features)
		{
			for (vk::Format format : candidates)
			{
				vk::FormatProperties props = physical_device->getFormatProperties(format);

				if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
				{
					return format;
				}
				else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
				{
					return format;
				}
			}

			throw std::runtime_error("failed to find supported format!");
		}

		vk::Format findDepthFormat(const PhysicalDevice& physical_device)
		{
			return findSupportedFormat(physical_device,
				{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
				vk::ImageTiling::eOptimal,
				vk::FormatFeatureFlagBits::eDepthStencilAttachment
			);
		}
	}

}
