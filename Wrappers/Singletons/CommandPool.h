#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <optional>

#include "PhysicalDevice.h"
#include "LogicalDevice.h"

namespace dmbrn
{
	/**
	 * \brief wrapper for vkCommandPool
	 */
	class CommandPool
	{
		friend struct Singletons;
	public:

		const vk::raii::CommandPool& operator*() const
		{
			return command_pool_;
		}

		const vk::raii::CommandPool* operator->() const
		{
			return &command_pool_;
		}

		vk::raii::CommandBuffer beginSingleTimeCommands(const LogicalDevice& device) const
		{
			const vk::CommandBufferAllocateInfo allocInfo
			{
				*command_pool_, vk::CommandBufferLevel::ePrimary, 1
			};

			vk::raii::CommandBuffer commandBuffer = std::move(device->allocateCommandBuffers(allocInfo).front());

			const vk::CommandBufferBeginInfo beginInfo
			{
				vk::CommandBufferUsageFlagBits::eOneTimeSubmit
			};

			commandBuffer.begin(beginInfo);

			return commandBuffer;
		}

		void endSingleTimeCommands(const vk::raii::Queue& gragraphics_queue,const vk::raii::CommandBuffer& commandBuffer) const
		{
			commandBuffer.end();

			const vk::SubmitInfo submitInfo
			{
				{}, {}, {}, 1, &*commandBuffer
			};

			gragraphics_queue.submit(submitInfo);
			gragraphics_queue.waitIdle();
		}

	private:
		vk::raii::CommandPool command_pool_;

		CommandPool(const PhysicalDevice& physical_device, const LogicalDevice& device):
			command_pool_(nullptr)
		{
			const PhysicalDevice::QueueFamilyIndices queueFamilyIndices = physical_device.getQueueFamilyIndices();

			const vk::CommandPoolCreateInfo poolInfo
			{
				vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
				queueFamilyIndices.graphicsFamily.value()
			};

			command_pool_ = vk::raii::CommandPool{device->createCommandPool(poolInfo)};
		}
	};
}
