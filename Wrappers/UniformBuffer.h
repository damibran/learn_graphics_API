#pragma once
#include "Singletons/PhysicalDevice.h"
#include "Singletons/LogicalDevice.h"


namespace dmbrn
{
	template <class UniformBufferObject>
	class UniformBuffer
	{
	public:
		~UniformBuffer() = default;
		UniformBuffer(const UniformBuffer&) = delete;

		UniformBuffer(UniformBuffer&&) = default;
		UniformBuffer& operator=(UniformBuffer&&) = default;

		UniformBuffer(const PhysicalDevice& physical_device, const LogicalDevice& device)
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
							physical_device.findMemoryType(
								uniform_buffers_[i].getMemoryRequirements().memoryTypeBits,
								vk::MemoryPropertyFlagBits::eHostVisible |
								vk::MemoryPropertyFlagBits::eHostCoherent)
						}
					));

				uniform_buffers_[i].bindMemory(*uniform_buffers_memory[i], 0);
			}
		}

		UniformBufferObject* mapMemory(int index)
		{
			return static_cast<UniformBufferObject*>(uniform_buffers_memory[index].mapMemory(
				0, sizeof(UniformBufferObject)));
		}

		void unmapMemory(int index)
		{
			uniform_buffers_memory[index].unmapMemory();
		}

		const vk::raii::Buffer& operator[](int index) const
		{
			return uniform_buffers_[index];
		}

	private:
		std::vector<vk::raii::Buffer> uniform_buffers_;
		std::vector<vk::raii::DeviceMemory> uniform_buffers_memory;
	};
};
