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

namespace dmbrn
{
	class CommandPool
	{
	public:
		CommandPool(const PhysicalDevice& physical_device, const LogicalDevice& device)
		{
			const PhysicalDevice::QueueFamilyIndices queueFamilyIndices = physical_device.getQueueFamilyIndices();

			const vk::CommandPoolCreateInfo poolInfo
			{
				vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
				queueFamilyIndices.graphicsFamily.value()
			};

			command_pool_ = std::make_unique<vk::raii::CommandPool>(device->createCommandPool(poolInfo));
		}

		const vk::raii::CommandPool& operator*() const
		{
			return *command_pool_;
		}

		const vk::raii::CommandPool* operator->() const
		{
			return command_pool_.get();
		}

		vk::raii::CommandBuffer beginSingleTimeCommands(const LogicalDevice& device)const
		{
			const vk::CommandBufferAllocateInfo allocInfo
			{
				**command_pool_, vk::CommandBufferLevel::ePrimary, 1
			};

			vk::raii::CommandBuffer commandBuffer = std::move(device->allocateCommandBuffers(allocInfo).front());

			const vk::CommandBufferBeginInfo beginInfo
			{
				vk::CommandBufferUsageFlagBits::eOneTimeSubmit
			};

			commandBuffer.begin(beginInfo);

			return commandBuffer;
		}

		void endSingleTimeCommands(vk::raii::Queue& gragraphics_queue, vk::raii::CommandBuffer& commandBuffer) const
		{
			commandBuffer.end();

			const vk::SubmitInfo submitInfo
			{
				{},{},{},1,&*commandBuffer
			};

			gragraphics_queue.submit(submitInfo);
			gragraphics_queue.waitIdle();
		}

	private:
		std::unique_ptr<vk::raii::CommandPool> command_pool_;

	};
}
