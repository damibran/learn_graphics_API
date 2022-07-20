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
	}

}
