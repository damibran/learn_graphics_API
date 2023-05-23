#pragma once
#include "Wrappers/Singletons/Singletons.h"


namespace dmbrn
{
	/**
	 * \brief represents CPU visible and coherent data buffer on GPU
	 * \tparam UniformBufferObject type of objects stored in buffer
	 */
	template <class UniformBufferObject>
	class UniformBuffer
	{
	public:
		~UniformBuffer() = default;
		UniformBuffer(const UniformBuffer&) = delete;

		UniformBuffer(UniformBuffer&&) = default;
		UniformBuffer& operator=(UniformBuffer&&) = default;

		UniformBuffer():
			uniform_buffer_(Singletons::device->createBuffer(
				vk::BufferCreateInfo{
					{}, sizeof(UniformBufferObject), vk::BufferUsageFlagBits::eUniformBuffer
				}
			)),
			uniform_buffer_memory(Singletons::device->allocateMemory(
				vk::MemoryAllocateInfo{
					uniform_buffer_.getMemoryRequirements().size,
					Singletons::physical_device.findMemoryType(
						uniform_buffer_.getMemoryRequirements().memoryTypeBits,
						vk::MemoryPropertyFlagBits::eHostVisible |
						vk::MemoryPropertyFlagBits::eHostCoherent)
				}
			))
		{
			uniform_buffer_.bindMemory(*uniform_buffer_memory, 0);
		}

		UniformBufferObject* mapMemory()
		{
			return static_cast<UniformBufferObject*>(uniform_buffer_memory.mapMemory(
				0, sizeof(UniformBufferObject)));
		}

		void unmapMemory()
		{
			uniform_buffer_memory.unmapMemory();
		}

		vk::raii::Buffer& operator*()
		{
			return uniform_buffer_;
		}

	private:
		vk::raii::Buffer uniform_buffer_;
		vk::raii::DeviceMemory uniform_buffer_memory;
	};
};
