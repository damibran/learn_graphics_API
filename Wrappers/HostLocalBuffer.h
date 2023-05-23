#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <Wrappers/Singletons/Singletons.h>

namespace dmbrn
{
	/**
	 * \brief represents host local GPU buffer
	 * \tparam T type of objects in buffer
	 * TODO actually this is device local buffer so should make renaming
	 */
	template <typename T>
	class HostLocalBuffer
	{
	public:
		HostLocalBuffer(): buffer_(nullptr), memory_(nullptr) // RAII violation !!!
		{
		}

		HostLocalBuffer(const HostLocalBuffer& other) = delete;
		const HostLocalBuffer& operator=(const HostLocalBuffer& other) = delete;

		HostLocalBuffer(HostLocalBuffer&& other) = default;
		HostLocalBuffer& operator=(HostLocalBuffer&& other) = default;

		const vk::Buffer getBuffer() const
		{
			return *buffer_;
		}

		HostLocalBuffer(const std::vector<T>& src_data, vk::BufferUsageFlagBits usage): buffer_(nullptr), memory_(nullptr) // RAII violation !!!
		{
			const vk::DeviceSize bufferSize = sizeof(T) * src_data.size();

			vk::BufferCreateInfo bufferInfo
			{
				{}, bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
				vk::SharingMode::eExclusive
			};
			vk::raii::Buffer stagingBuffer = Singletons::device->createBuffer(bufferInfo);

			const vk::MemoryAllocateInfo memory_allocate_info
			{
				stagingBuffer.getMemoryRequirements().size,
				Singletons::physical_device.findMemoryType(stagingBuffer.getMemoryRequirements().memoryTypeBits,
				                                           vk::MemoryPropertyFlagBits::eHostVisible |
				                                           vk::MemoryPropertyFlagBits::eHostCoherent)
			};

			const vk::raii::DeviceMemory stagingBufferMemory = Singletons::device->allocateMemory(memory_allocate_info);

			stagingBuffer.bindMemory(*stagingBufferMemory, 0);

			void* data;
			data = stagingBufferMemory.mapMemory(0, bufferSize, {});
			memcpy(data, src_data.data(), bufferSize);
			stagingBufferMemory.unmapMemory();

			bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst | usage;
			buffer_ = vk::raii::Buffer{Singletons::device->createBuffer(bufferInfo)};

			const auto allocate_info = vk::MemoryAllocateInfo
			{
				buffer_.getMemoryRequirements().size,
				Singletons::physical_device.findMemoryType(buffer_.getMemoryRequirements().memoryTypeBits,
				                                           vk::MemoryPropertyFlagBits::eDeviceLocal)
			};

			memory_ = vk::raii::DeviceMemory{Singletons::device->allocateMemory(allocate_info)};

			buffer_.bindMemory(*memory_, 0);

			copyBuffer(stagingBuffer, buffer_, bufferSize);
		}

	private:
		void copyBuffer(const vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size)
		{
			vk::raii::CommandBuffer commandBuffer = Singletons::command_pool.
				beginSingleTimeCommands(Singletons::device);

			const vk::BufferCopy copyRegion
			{
				0, 0, size
			};
			commandBuffer.copyBuffer(*srcBuffer, *dstBuffer, copyRegion);

			Singletons::command_pool.endSingleTimeCommands(Singletons::graphics_queue, commandBuffer);
		}

		vk::raii::Buffer buffer_;
		vk::raii::DeviceMemory memory_;
	};
}
