#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <fstream>
#include <optional>
#include <set>

#include <glm/glm.hpp>

#include "GLFWwindowWrapper.h"
#include "Instance.h"
#include "Surface.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "DescriptorSetLayout.h"
#include "GraphicsPipeline.h"
#include "FrameBuffers.h"

namespace dmbrn
{
	class CommandPool
	{
	public:
		CommandPool(const PhysicalDevice& physical_device, const LogicalDevice& device)
		{
			PhysicalDevice::QueueFamilyIndices queueFamilyIndices = physical_device.getQueueFamilyIndices();

			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

			command_pool_ = std::make_unique<vk::raii::CommandPool>(device->createCommandPool(poolInfo));
		}

		const vk::raii::CommandPool* operator->() const
		{
			return command_pool_.get();
		}

	private:
		std::unique_ptr<vk::raii::CommandPool> command_pool_;

	};
}
