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
	class Texture // need to cache current layout it makes transitioning clearer 
	{
	public:
		Texture() :
			texture_image(nullptr),
			texture_image_memory_(nullptr),
			image_view_(nullptr),
			sampler_(nullptr)
		{
		}

		Texture(vk::Extent2D extent, const PhysicalDevice& physical_device, const LogicalDevice& device,
			const CommandPool& command_pool, const vk::raii::Queue& gragraphics_queue) :
			texture_image(nullptr),
			texture_image_memory_(nullptr),
			image_view_(nullptr),
			sampler_(nullptr)
		{
			createTextureImageWithSize(extent, physical_device, device, command_pool, gragraphics_queue);
			createTextureImageView(device);
			createTextureSampler(device, physical_device);
		}

		Texture(const std::string& texPath, const PhysicalDevice& physical_device, const LogicalDevice& device,
			const CommandPool& command_pool, vk::raii::Queue gragraphics_queue) :
			texture_image(nullptr),
			texture_image_memory_(nullptr),
			image_view_(nullptr),
			sampler_(nullptr)
		{
			createTextureImageFromFile(texPath, physical_device, device, command_pool, gragraphics_queue);
			createTextureImageView(device);
			createTextureSampler(device, physical_device);
		}

		void transitionImageLayoutWithCommandBuffer(const vk::raii::CommandBuffer& command_buffer,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout) const
		{
			vk::ImageMemoryBarrier barrier
			{
				{}, {}, oldLayout, newLayout,
				VK_QUEUE_FAMILY_IGNORED,VK_QUEUE_FAMILY_IGNORED, *texture_image,
				vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
			};

			vk::PipelineStageFlags sourceStage;
			vk::PipelineStageFlags destinationStage;

			checkTransitions(barrier, sourceStage, destinationStage, oldLayout, newLayout);

			command_buffer.pipelineBarrier(sourceStage, destinationStage, {}
			, nullptr, nullptr, barrier);
		}

		const vk::raii::ImageView& getImageView() const
		{
			return image_view_;
		}

		const vk::raii::Sampler& getSampler() const
		{
			return sampler_;
		}

	private:
		vk::raii::Image texture_image;
		vk::raii::DeviceMemory texture_image_memory_;
		vk::raii::ImageView image_view_;
		vk::raii::Sampler sampler_;

		void createTextureImageWithSize(vk::Extent2D extent, const PhysicalDevice& physical_device,
			const LogicalDevice& device, const CommandPool& command_pool,
			const vk::raii::Queue& gragraphics_queue)
		{
			createImage(device, physical_device, extent, vk::Format::eR8G8B8A8Srgb,
				vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
				vk::MemoryPropertyFlagBits::eDeviceLocal,
				texture_image, texture_image_memory_);

			singleTimeTransitionImageLayout(device, command_pool, gragraphics_queue, texture_image,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eShaderReadOnlyOptimal);
		}

		void createTextureImageFromFile(const std::string& texPath, const PhysicalDevice& physical_device,
			const LogicalDevice& device, const CommandPool& command_pool,
			vk::raii::Queue gragraphics_queue)
		{
			int texWidth, texHeight, texChannels;
			stbi_uc* pixels = stbi_load(texPath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			const vk::DeviceSize imageSize = texWidth * texHeight * 4;

			if (!pixels)
			{
				throw std::runtime_error("failed to load texture image!");
			}

			const vk::BufferCreateInfo bufferInfo
			{
				{}, imageSize, vk::BufferUsageFlagBits::eTransferSrc,
				vk::SharingMode::eExclusive
			};

			vk::raii::Buffer stagingBuffer = device->createBuffer(bufferInfo);

			const vk::MemoryRequirements memRequirements = stagingBuffer.getMemoryRequirements();

			const vk::MemoryAllocateInfo allocInfo
			{
				memRequirements.size,
				physical_device.findMemoryType(memRequirements.memoryTypeBits,
											   vk::MemoryPropertyFlagBits::eHostVisible |
											   vk::MemoryPropertyFlagBits::eHostCoherent)
			};

			const vk::raii::DeviceMemory stagingBufferMemory = device->allocateMemory(allocInfo);

			stagingBuffer.bindMemory(*stagingBufferMemory, 0);

			void* data = stagingBufferMemory.mapMemory(0, imageSize);
			memcpy(data, pixels, imageSize);
			stagingBufferMemory.unmapMemory();

			stbi_image_free(pixels);

			createImage(device, physical_device, { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight) },
				vk::Format::eR8G8B8A8Srgb,
				vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
				vk::MemoryPropertyFlagBits::eDeviceLocal,
				texture_image, texture_image_memory_);

			singleTimeTransitionImageLayout(device, command_pool, gragraphics_queue, texture_image,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eTransferDstOptimal);
			copyBufferToImage(device, command_pool, gragraphics_queue, stagingBuffer, texture_image,
				static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
			singleTimeTransitionImageLayout(device, command_pool, gragraphics_queue, texture_image,
				vk::ImageLayout::eTransferDstOptimal,
				vk::ImageLayout::eShaderReadOnlyOptimal);
		}

		void createImage(const LogicalDevice& device, const PhysicalDevice& physical_device, vk::Extent2D extent,
			vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
			vk::MemoryPropertyFlags properties, vk::raii::Image& image,
			vk::raii::DeviceMemory& imageMemory) const
		{
			const vk::ImageCreateInfo imageInfo
			{
				{}, vk::ImageType::e2D,
				format,
				vk::Extent3D{extent, 1},
				1, 1, vk::SampleCountFlagBits::e1,
				tiling, usage, vk::SharingMode::eExclusive, {},
				{}, vk::ImageLayout::eUndefined
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

		void singleTimeTransitionImageLayout(const LogicalDevice& device, const CommandPool& command_pool,
			const vk::raii::Queue& gragraphics_queue,
			vk::raii::Image& image, vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout)
		{
			vk::raii::CommandBuffer commandBuffer = command_pool.beginSingleTimeCommands(device);

			vk::ImageMemoryBarrier barrier
			{
				{}, {}, oldLayout, newLayout,
				VK_QUEUE_FAMILY_IGNORED,VK_QUEUE_FAMILY_IGNORED, *image,
				vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
			};

			vk::PipelineStageFlags sourceStage;
			vk::PipelineStageFlags destinationStage;

			checkTransitions(barrier, sourceStage, destinationStage, oldLayout, newLayout);

			commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}
			, nullptr, nullptr, barrier);

			command_pool.endSingleTimeCommands(gragraphics_queue, commandBuffer);
		}

		void checkTransitions(vk::ImageMemoryBarrier& barrier, vk::PipelineStageFlags& sourceStage,
			vk::PipelineStageFlags& destinationStage, vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout) const
		{
			if (oldLayout == vk::ImageLayout::eUndefined)
			{
				barrier.srcAccessMask = vk::AccessFlagBits::eNone;
				sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;

				if (newLayout == vk::ImageLayout::eTransferDstOptimal)
				{
					barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
					destinationStage = vk::PipelineStageFlagBits::eTransfer;
				}
				else if (newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
				{
					barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
					destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
				}
			}
			else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
				newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
			{
				barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
				barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

				sourceStage = vk::PipelineStageFlagBits::eTransfer;
				destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
			}
			else if (oldLayout == vk::ImageLayout::eColorAttachmentOptimal &&
				newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
			{
				barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
				barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

				sourceStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
				destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
			}
			else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal &&
				newLayout == vk::ImageLayout::eColorAttachmentOptimal)
			{
				barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
				barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

				sourceStage = vk::PipelineStageFlagBits::eFragmentShader;
				destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			}
			else
			{
				throw std::invalid_argument("unsupported layout transition!");
			}
		}

		void copyBufferToImage(const LogicalDevice& device, const CommandPool& command_pool,
			vk::raii::Queue gragraphics_queue,
			vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height)
		{
			vk::raii::CommandBuffer commandBuffer = command_pool.beginSingleTimeCommands(device);

			vk::BufferImageCopy region
			{
				0, 0, 0,
				vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1},
				vk::Offset3D{0, 0, 0},
				vk::Extent3D{width, height, 1}
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
				vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
			};

			image_view_ = vk::raii::ImageView{ device->createImageView(viewInfo) };
		}

		void createTextureSampler(const LogicalDevice& device, const PhysicalDevice& physical_device)
		{
			const vk::PhysicalDeviceProperties properties = physical_device->getProperties();

			const vk::SamplerCreateInfo samplerInfo
			{
				{}, vk::Filter::eLinear, vk::Filter::eLinear,
				vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eRepeat,
				vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
				{},VK_TRUE, properties.limits.maxSamplerAnisotropy,
				VK_FALSE, vk::CompareOp::eAlways, {}, {}, vk::BorderColor::eIntOpaqueBlack,
				VK_FALSE
			};
			sampler_ = vk::raii::Sampler{ device->createSampler(samplerInfo) };
		}
	};
}
