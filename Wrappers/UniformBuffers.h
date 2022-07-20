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
#include <stb_image.h>

#include "GLFWwindowWrapper.h"
#include "Instance.h"
#include "Surface.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "DescriptorSetLayout.h"
#include "GraphicsPipeline.h"
#include "CommandPool.h"
#include "Texture.h"
#include "VertexIndexBuffers.h"

namespace dmbrn
{
	class UniformBuffers
	{
	public:
		struct UniformBufferObject {
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};

		UniformBuffers(const PhysicalDevice& physical_device, const LogicalDevice& device)
		{
			VkDeviceSize bufferSize = sizeof(UniformBufferObject);

			for (size_t i = 0; i < device.MAX_FRAMES_IN_FLIGHT; i++) {

				uniform_buffers_.push_back(
					device->createBuffer(
						vk::BufferCreateInfo{
							{},bufferSize,vk::BufferUsageFlagBits::eUniformBuffer
						}
				));

				uniform_buffers_memory.push_back(
					device->allocateMemory(
						vk::MemoryAllocateInfo{ uniform_buffers_[i].getMemoryRequirements().size,
						physical_device.findMemoryType(uniform_buffers_[i].getMemoryRequirements().memoryTypeBits,
							vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) }
				));

				uniform_buffers_[i].bindMemory(*uniform_buffers_memory[i], 0);
			}
		}

		const vk::raii::Buffer& operator[](int index)const
		{
			return uniform_buffers_[index];
		}

		const vk::raii::DeviceMemory& getUBMemory(int index)const
		{
			return uniform_buffers_memory[index];
		}

	private:
		std::vector<vk::raii::Buffer> uniform_buffers_;
		std::vector<vk::raii::DeviceMemory> uniform_buffers_memory;
	};
}
