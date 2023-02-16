#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <glm/glm.hpp>

#include "Wrappers/Singletons/PhysicalDevice.h"
#include "Wrappers/Singletons/LogicalDevice.h"

namespace dmbrn
{
	class DiffusionUniformBuffer
	{
	public:
		~DiffusionUniformBuffer()=default;
		DiffusionUniformBuffer(const DiffusionUniformBuffer& )=delete;

		DiffusionUniformBuffer(DiffusionUniformBuffer&& )=default;
		DiffusionUniformBuffer& operator=(DiffusionUniformBuffer&&)=default;

		struct UniformBufferObject
		{
			alignas(16) glm::vec4 base_color;
		};

		DiffusionUniformBuffer(const PhysicalDevice& physical_device, const LogicalDevice& device, const glm::vec4& base_color)
		{
			const vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

			for (size_t i = 0; i < device.MAX_FRAMES_IN_FLIGHT; i++)
			{
				uniform_buffers_.push_back(
					device->createBuffer(
						vk::BufferCreateInfo{
							{}, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer
						}
					));

				uniform_buffers_memory.push_back(
					device->allocateMemory(
						vk::MemoryAllocateInfo{
							uniform_buffers_[i].getMemoryRequirements().size,
							physical_device.findMemoryType(uniform_buffers_[i].getMemoryRequirements().memoryTypeBits,
							                               vk::MemoryPropertyFlagBits::eHostVisible |
							                               vk::MemoryPropertyFlagBits::eHostCoherent)
						}
					));

				uniform_buffers_[i].bindMemory(*uniform_buffers_memory[i], 0);

				UniformBufferObject* ubo = static_cast<UniformBufferObject*>(uniform_buffers_memory[i].mapMemory(0, sizeof(UniformBufferObject)));
				ubo->base_color=base_color;
				uniform_buffers_memory[i].unmapMemory();
			}
		}

		const vk::raii::Buffer& operator[](int index) const
		{
			return uniform_buffers_[index];
		}

		const vk::raii::DeviceMemory& getUBMemory(int index) const
		{
			return uniform_buffers_memory[index];
		}

	private:
		std::vector<vk::raii::Buffer> uniform_buffers_;
		std::vector<vk::raii::DeviceMemory> uniform_buffers_memory;
	};
}
