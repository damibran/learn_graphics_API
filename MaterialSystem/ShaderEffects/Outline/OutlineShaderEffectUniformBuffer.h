#pragma once

#include <glm/glm.hpp>
#include "Wrappers/Singletons/PhysicalDevice.h"
#include "Wrappers/Singletons/LogicalDevice.h"


namespace dmbrn
{
	class OutlineShaderEffectUniformBuffer
	{
	public:
		~OutlineShaderEffectUniformBuffer()=default;
		OutlineShaderEffectUniformBuffer(const OutlineShaderEffectUniformBuffer& )=delete;

		OutlineShaderEffectUniformBuffer(OutlineShaderEffectUniformBuffer&& )=default;
		OutlineShaderEffectUniformBuffer& operator=(OutlineShaderEffectUniformBuffer&&)=default;

		struct UniformBufferObject
		{
			alignas(16) glm::vec3 color;
			alignas(16) float scale_factor;
		};

		OutlineShaderEffectUniformBuffer(const PhysicalDevice& physical_device, const LogicalDevice& device)
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
