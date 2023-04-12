#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>
#include "Wrappers/Singletons/GLFWwindowWrapper.h"
#include "Wrappers/Singletons/PhysicalDevice.h"

namespace dmbrn::utils
{
	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

	[[nodiscard]] std::string matrix_to_str_row_maj(const glm::mat4& m);

	vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const GLFWwindowWrapper& window);

	vk::Format findSupportedFormat(const PhysicalDevice& physical_device, const std::vector<vk::Format>& candidates,
	                               vk::ImageTiling tiling, vk::FormatFeatureFlags features);

	vk::Format findDepthFormat(const PhysicalDevice& physical_device);

	uint32_t capabilitiesGetImageCount(const PhysicalDevice& physical_device, const Surface& surface);
}
