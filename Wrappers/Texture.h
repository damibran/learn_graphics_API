#pragma once
#include <stb_image.h>

#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <limits>

#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "CommandPool.h"

namespace dmbrn
{
	class Texture
	{
	public:
		Texture(const PhysicalDevice& physical_device, const LogicalDevice& device,
			const CommandPool& command_pool, vk::raii::Queue gragraphics_queue) :
			texture_image(nullptr),
			texture_image_memory_(nullptr),
			image_view_(nullptr),
			sampler_(nullptr)
		{
			createTextureImage(physical_device, device, command_pool, gragraphics_queue);
			createTextureImageView(device);
			createTextureSampler(device, physical_device);
		}

		vk::ImageLayout getLayout()const
		{
			return vk::ImageLayout::eShaderReadOnlyOptimal;
		}

		const vk::raii::ImageView& getImageView()const
		{
			return image_view_;
		}

		const vk::raii::Sampler& getSampler()const
		{
			return sampler_;
		}

	private:
		vk::raii::Image texture_image;
		vk::raii::DeviceMemory texture_image_memory_;
		vk::raii::ImageView image_view_;
		vk::raii::Sampler sampler_;

		void createTextureImage(const PhysicalDevice& physical_device, const LogicalDevice& device, const CommandPool& command_pool, vk::raii::Queue gragraphics_queue)
		{
			int texWidth, texHeight, texChannels;
			stbi_uc* pixels = stbi_load("Textures/Tutorial/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			const vk::DeviceSize imageSize = texWidth * texHeight * 4;

			if (!pixels) {
				throw std::runtime_error("failed to load texture image!");
			}

			const vk::BufferCreateInfo bufferInfo
			{
				{},imageSize,vk::BufferUsageFlagBits::eTransferSrc,
				vk::SharingMode::eExclusive
			};

			vk::raii::Buffer stagingBuffer = device->createBuffer(bufferInfo);

			const vk::MemoryRequirements memRequirements = stagingBuffer.getMemoryRequirements();

			const vk::MemoryAllocateInfo allocInfo
			{
				memRequirements.size,
				physical_device.findMemoryType(memRequirements.memoryTypeBits,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
			};

			const vk::raii::DeviceMemory stagingBufferMemory = device->allocateMemory(allocInfo);

			stagingBuffer.bindMemory(*stagingBufferMemory, 0);

			void* data = stagingBufferMemory.mapMemory(0, imageSize);
			memcpy(data, pixels, static_cast<size_t>(imageSize));
			stagingBufferMemory.unmapMemory();

			stbi_image_free(pixels);

			createImage(device, physical_device, texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
				vk::MemoryPropertyFlagBits::eDeviceLocal,
				texture_image, texture_image_memory_);

			transitionImageLayout(device, command_pool, gragraphics_queue, texture_image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined,
				vk::ImageLayout::eTransferDstOptimal);
			copyBufferToImage(device, command_pool, gragraphics_queue, stagingBuffer, texture_image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
			transitionImageLayout(device, command_pool, gragraphics_queue, texture_image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal,
				vk::ImageLayout::eShaderReadOnlyOptimal);
		}

		void createImage(const LogicalDevice& device, const PhysicalDevice& physical_device, uint32_t width, uint32_t height,
			vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image& image,
			vk::raii::DeviceMemory& imageMemory) const
		{
			const vk::ImageCreateInfo imageInfo
			{
				{}, vk::ImageType::e2D,
				format,
				vk::Extent3D{width,height,1},
				1, 1, vk::SampleCountFlagBits::e1,
				tiling,usage,vk::SharingMode::eExclusive,{},
				{},vk::ImageLayout::eUndefined
			};

			image = vk::raii::Image{ device->createImage(imageInfo) };

			const vk::MemoryRequirements memRequirements = image.getMemoryRequirements();

			const vk::MemoryAllocateInfo allocInfo
			{
				memRequirements.size,
				physical_device.findMemoryType(memRequirements.memoryTypeBits, properties)
			};

			imageMemory = vk::raii::DeviceMemory{ device->allocateMemory(allocInfo) };

			image.bindMemory(*imageMemory, 0);
		}

		void transitionImageLayout(const LogicalDevice& device, const CommandPool& command_pool, vk::raii::Queue gragraphics_queue,
			vk::raii::Image& image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
		{
			vk::raii::CommandBuffer commandBuffer = command_pool.beginSingleTimeCommands(device);

			vk::ImageMemoryBarrier barrier
			{
				{},{},oldLayout, newLayout,
				VK_QUEUE_FAMILY_IGNORED,VK_QUEUE_FAMILY_IGNORED, *image,
				vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor,0,1,0,1}
			};

			vk::PipelineStageFlags sourceStage;
			vk::PipelineStageFlags destinationStage;

			if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
				barrier.srcAccessMask = vk::AccessFlagBits::eNone;
				barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

				sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
				destinationStage = vk::PipelineStageFlagBits::eTransfer;;
			}
			else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
				barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
				barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

				sourceStage = vk::PipelineStageFlagBits::eTransfer;
				destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
			}
			else {
				throw std::invalid_argument("unsupported layout transition!");
			}

			commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}
			, nullptr, nullptr, barrier);

			command_pool.endSingleTimeCommands(gragraphics_queue, commandBuffer);
		}


		void copyBufferToImage(const LogicalDevice& device, const CommandPool& command_pool, vk::raii::Queue gragraphics_queue,
			vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height)
		{
			vk::raii::CommandBuffer commandBuffer = command_pool.beginSingleTimeCommands(device);

			vk::BufferImageCopy region
			{
				0,0,0,
				vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor,0,0,1},
				vk::Offset3D{ 0, 0, 0 },
				vk::Extent3D{width, height,1}
			};

			commandBuffer.copyBufferToImage(*buffer, *image, vk::ImageLayout::eTransferDstOptimal, region);

			command_pool.endSingleTimeCommands(gragraphics_queue, commandBuffer);
		}

		void createTextureImageView(const LogicalDevice& device)
		{
			const vk::ImageViewCreateInfo viewInfo
			{
				{}, *texture_image,
				vk::ImageViewType::e2D,
				vk::Format::eR8G8B8A8Srgb,
				{},
				vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor,0,1,0,1}
			};

			image_view_ = vk::raii::ImageView{ device->createImageView(viewInfo) };
		}

		void createTextureSampler(const LogicalDevice& device, const PhysicalDevice& physical_device)
		{
			const vk::PhysicalDeviceProperties properties = physical_device->getProperties();

			const vk::SamplerCreateInfo samplerInfo
			{
				{}, vk::Filter::eLinear,vk::Filter::eLinear,
				vk::SamplerMipmapMode::eLinear,vk::SamplerAddressMode::eRepeat,
				vk::SamplerAddressMode::eRepeat,vk::SamplerAddressMode::eRepeat,
				{},VK_TRUE,properties.limits.maxSamplerAnisotropy,
				VK_FALSE,vk::CompareOp::eAlways,{},{},vk::BorderColor::eIntOpaqueBlack,
				VK_FALSE
			};
			sampler_ = vk::raii::Sampler{ device->createSampler(samplerInfo) };
		}
	};
}
