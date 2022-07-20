#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <glm/glm.hpp>

#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "GraphicsPipeline.h"
#include "CommandPool.h"

namespace dmbrn
{
	class VertexIndexBuffers
	{
	public:

		VertexIndexBuffers(const PhysicalDevice& physical_device, const LogicalDevice& device, const CommandPool& command_pool, vk::raii::Queue gragraphics_queue)
		{
			createVertexBuffer(physical_device, device, command_pool, gragraphics_queue);
			createIndexBuffer(physical_device, device, command_pool, gragraphics_queue);
		}

		const std::vector<Vertex> vertices = {
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
		};

		const std::vector<uint16_t> indices = {
			0, 1, 2, 2, 3, 0
		};

		const vk::raii::Buffer& getVertex()const
		{
			return *vertex_buffer_;
		}

		const vk::raii::Buffer& getIndex()const
		{
			return *index_buffer_;
		}

		int getIndeciesCount()const
		{
			return indices.size();
		}

	private:
		std::unique_ptr<vk::raii::Buffer> vertex_buffer_;
		std::unique_ptr<vk::raii::DeviceMemory> vertex_buffer_memory_;

		std::unique_ptr<vk::raii::Buffer> index_buffer_;
		std::unique_ptr<vk::raii::DeviceMemory> index_buffer_memory_;


		void createVertexBuffer(const PhysicalDevice& physical_device, const LogicalDevice& device, const CommandPool& command_pool, const vk::raii::Queue& gragraphics_queue)
		{
			const vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

			vk::BufferCreateInfo bufferInfo
			{
				{},bufferSize,vk::BufferUsageFlagBits::eTransferSrc,
				vk::SharingMode::eExclusive
			};
			vk::raii::Buffer stagingBuffer = device->createBuffer(bufferInfo);

			const vk::MemoryAllocateInfo memory_allocate_info
			{
				stagingBuffer.getMemoryRequirements().size,
				physical_device.findMemoryType(stagingBuffer.getMemoryRequirements().memoryTypeBits,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
			};

			const vk::raii::DeviceMemory stagingBufferMemory = device->allocateMemory(memory_allocate_info);

			stagingBuffer.bindMemory(*stagingBufferMemory, 0);

			void* data;
			data = stagingBufferMemory.mapMemory(0, bufferSize, {});
			memcpy(data, vertices.data(), (size_t)bufferSize);
			stagingBufferMemory.unmapMemory();

			bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
			vertex_buffer_ = std::make_unique<vk::raii::Buffer>(device->createBuffer(bufferInfo));

			const vk::MemoryAllocateInfo allocate_info = vk::MemoryAllocateInfo
			{
				vertex_buffer_->getMemoryRequirements().size,
				physical_device.findMemoryType(vertex_buffer_->getMemoryRequirements().memoryTypeBits,
				vk::MemoryPropertyFlagBits::eDeviceLocal)
			};

			vertex_buffer_memory_ = std::make_unique<vk::raii::DeviceMemory>(device->allocateMemory(allocate_info));

			vertex_buffer_->bindMemory(**vertex_buffer_memory_, 0);

			copyBuffer(device, command_pool, gragraphics_queue, stagingBuffer, *vertex_buffer_, bufferSize);
		}

		void createIndexBuffer(const PhysicalDevice& physical_device, const LogicalDevice& device, const CommandPool& command_pool, vk::raii::Queue gragraphics_queue)
		{
			const vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

			vk::BufferCreateInfo bufferInfo
			{
				{}, bufferSize,vk::BufferUsageFlagBits::eTransferSrc,
				vk::SharingMode::eExclusive
			};
			vk::raii::Buffer stagingBuffer = device->createBuffer(bufferInfo);

			const vk::MemoryAllocateInfo memory_allocate_info
			{
				stagingBuffer.getMemoryRequirements().size,
				physical_device.findMemoryType(stagingBuffer.getMemoryRequirements().memoryTypeBits,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
			};

			const vk::raii::DeviceMemory stagingBufferMemory = device->allocateMemory(memory_allocate_info);

			stagingBuffer.bindMemory(*stagingBufferMemory, 0);

			void* data = stagingBufferMemory.mapMemory(0, bufferSize, {});
			memcpy(data, indices.data(), (size_t)bufferSize);
			stagingBufferMemory.unmapMemory();

			bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
			index_buffer_ = std::make_unique<vk::raii::Buffer>(device->createBuffer(bufferInfo));

			const vk::MemoryAllocateInfo allocate_info
			{
				index_buffer_->getMemoryRequirements().size,
				physical_device.findMemoryType(index_buffer_->getMemoryRequirements().memoryTypeBits,
					vk::MemoryPropertyFlagBits::eDeviceLocal)
			};
			index_buffer_memory_ = std::make_unique<vk::raii::DeviceMemory>(device->allocateMemory(allocate_info));

			index_buffer_->bindMemory(**index_buffer_memory_, 0);

			copyBuffer(device, command_pool, gragraphics_queue, stagingBuffer, *index_buffer_, bufferSize);
		}

		void copyBuffer(const LogicalDevice& device, const CommandPool& command_pool, vk::raii::Queue gragraphics_queue, 
			vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size)
		{
			vk::raii::CommandBuffer commandBuffer = command_pool.beginSingleTimeCommands(device);

			const vk::BufferCopy copyRegion
			{
				0,0,size
			};
			commandBuffer.copyBuffer(*srcBuffer, *dstBuffer, copyRegion);

			command_pool.endSingleTimeCommands(gragraphics_queue, commandBuffer);
		}
	};
}
