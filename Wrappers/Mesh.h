#pragma once

#include <string>
#include <vector>

#include "Texture.h"
#include "Vertex.h"

namespace dmbrn
{
	class Mesh
	{
	public:
		// mesh Data
		std::vector<Vertex> vertices_;
		std::vector<uint16_t> indices_;
		std::vector<Texture> textures_;

		// constructor
		Mesh(std::vector<Vertex>&& vertices, std::vector<uint16_t>&& indices, std::vector<Texture>&& textures) :
			vertices_(std::move(vertices)),
			indices_(std::move(indices)),
			textures_(std::move(textures)),
			vertex_buffer_(nullptr),
			index_buffer_(nullptr),
			vertex_buffer_memory_(nullptr),
			index_buffer_memory_(nullptr)
		{
			// now that we have all the required data, set the vertex buffers and its attribute pointers.
			createVertexBuffer(Singletons::physical_device, Singletons::device, Singletons::command_pool,
			                   Singletons::graphics_queue);
			createIndexBuffer(Singletons::physical_device, Singletons::device, Singletons::command_pool,
			                  Singletons::graphics_queue);
		}


		vk::raii::Buffer vertex_buffer_;
		vk::raii::Buffer index_buffer_;


	private:
		// render data
		vk::raii::DeviceMemory vertex_buffer_memory_;

		vk::raii::DeviceMemory index_buffer_memory_;
		// initializes all the buffer objects/arrays
		void createVertexBuffer(const PhysicalDevice& physical_device, const LogicalDevice& device,
		                        const CommandPool& command_pool, const vk::raii::Queue& gragraphics_queue)
		{
			const vk::DeviceSize bufferSize = sizeof(vertices_[0]) * vertices_.size();

			vk::BufferCreateInfo bufferInfo
			{
				{}, bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
				vk::SharingMode::eExclusive
			};
			vk::raii::Buffer stagingBuffer = device->createBuffer(bufferInfo);

			const vk::MemoryAllocateInfo memory_allocate_info
			{
				stagingBuffer.getMemoryRequirements().size,
				physical_device.findMemoryType(stagingBuffer.getMemoryRequirements().memoryTypeBits,
				                               vk::MemoryPropertyFlagBits::eHostVisible |
				                               vk::MemoryPropertyFlagBits::eHostCoherent)
			};

			const vk::raii::DeviceMemory stagingBufferMemory = device->allocateMemory(memory_allocate_info);

			stagingBuffer.bindMemory(*stagingBufferMemory, 0);

			void* data;
			data = stagingBufferMemory.mapMemory(0, bufferSize, {});
			memcpy(data, vertices_.data(), bufferSize);
			stagingBufferMemory.unmapMemory();

			bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
			vertex_buffer_ = vk::raii::Buffer{device->createBuffer(bufferInfo)};

			const auto allocate_info = vk::MemoryAllocateInfo
			{
				vertex_buffer_.getMemoryRequirements().size,
				physical_device.findMemoryType(vertex_buffer_.getMemoryRequirements().memoryTypeBits,
				                               vk::MemoryPropertyFlagBits::eDeviceLocal)
			};

			vertex_buffer_memory_ = vk::raii::DeviceMemory{device->allocateMemory(allocate_info)};

			vertex_buffer_.bindMemory(*vertex_buffer_memory_, 0);

			copyBuffer(device, command_pool, gragraphics_queue, stagingBuffer, vertex_buffer_, bufferSize);
		}

		void createIndexBuffer(const PhysicalDevice& physical_device, const LogicalDevice& device,
		                       const CommandPool& command_pool, vk::raii::Queue gragraphics_queue)
		{
			const vk::DeviceSize bufferSize = sizeof(indices_[0]) * indices_.size();

			vk::BufferCreateInfo bufferInfo
			{
				{}, bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
				vk::SharingMode::eExclusive
			};
			vk::raii::Buffer stagingBuffer = device->createBuffer(bufferInfo);

			const vk::MemoryAllocateInfo memory_allocate_info
			{
				stagingBuffer.getMemoryRequirements().size,
				physical_device.findMemoryType(stagingBuffer.getMemoryRequirements().memoryTypeBits,
				                               vk::MemoryPropertyFlagBits::eHostVisible |
				                               vk::MemoryPropertyFlagBits::eHostCoherent)
			};

			const vk::raii::DeviceMemory stagingBufferMemory = device->allocateMemory(memory_allocate_info);

			stagingBuffer.bindMemory(*stagingBufferMemory, 0);

			void* data = stagingBufferMemory.mapMemory(0, bufferSize, {});
			memcpy(data, indices_.data(), bufferSize);
			stagingBufferMemory.unmapMemory();

			bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
			index_buffer_ = vk::raii::Buffer{device->createBuffer(bufferInfo)};

			const vk::MemoryAllocateInfo allocate_info
			{
				index_buffer_.getMemoryRequirements().size,
				physical_device.findMemoryType(index_buffer_.getMemoryRequirements().memoryTypeBits,
				                               vk::MemoryPropertyFlagBits::eDeviceLocal)
			};
			index_buffer_memory_ = vk::raii::DeviceMemory{device->allocateMemory(allocate_info)};

			index_buffer_.bindMemory(*index_buffer_memory_, 0);

			copyBuffer(device, command_pool, gragraphics_queue, stagingBuffer, index_buffer_, bufferSize);
		}

		void copyBuffer(const LogicalDevice& device, const CommandPool& command_pool, vk::raii::Queue gragraphics_queue,
		                vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size)
		{
			vk::raii::CommandBuffer commandBuffer = command_pool.beginSingleTimeCommands(device);

			const vk::BufferCopy copyRegion
			{
				0, 0, size
			};
			commandBuffer.copyBuffer(*srcBuffer, *dstBuffer, copyRegion);

			command_pool.endSingleTimeCommands(gragraphics_queue, commandBuffer);
		}
	};
}
