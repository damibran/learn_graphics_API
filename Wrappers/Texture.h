#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stb_image.h>

#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <limits>

#include "GLFWwindowWrapper.h"
#include "Instance.h"
#include "Surface.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "DescriptorSetLayout.h"
#include "GraphicsPipeline.h"
#include "FrameBuffers.h"
#include "CommandPool.h"

namespace dmbrn
{
	class Texture
	{
	public:
		Texture(const PhysicalDevice& physical_device, const LogicalDevice& device, const CommandPool& command_pool, vk::raii::Queue gragraphics_queue)
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
			return *image_view_;
		}

		const vk::raii::Sampler& getSampler()const
		{
			return *sampler_;
		}

	private:
		std::unique_ptr<vk::raii::Image> texture_image;
		std::unique_ptr<vk::raii::DeviceMemory> texture_image_memory_;
		std::unique_ptr<vk::raii::ImageView> image_view_;
		std::unique_ptr<vk::raii::Sampler> sampler_;

		void createTextureImage(const PhysicalDevice& physical_device, const LogicalDevice& device, const CommandPool& command_pool, vk::raii::Queue gragraphics_queue)
		{
			int texWidth, texHeight, texChannels;
			stbi_uc* pixels = stbi_load("Textures/Tutorial/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			vk::DeviceSize imageSize = texWidth * texHeight * 4;

			if (!pixels) {
				throw std::runtime_error("failed to load texture image!");
			}

			vk::BufferCreateInfo bufferInfo{};
			bufferInfo.size = imageSize;
			bufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
			bufferInfo.sharingMode = vk::SharingMode::eExclusive;

			vk::raii::Buffer stagingBuffer = device->createBuffer(bufferInfo);

			vk::MemoryRequirements memRequirements = stagingBuffer.getMemoryRequirements();

			vk::MemoryAllocateInfo allocInfo{};
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = physical_device.findMemoryType(memRequirements.memoryTypeBits,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

			vk::raii::DeviceMemory stagingBufferMemory = device->allocateMemory(allocInfo);

			stagingBuffer.bindMemory(*stagingBufferMemory, 0);

			void* data;
			data = stagingBufferMemory.mapMemory(0, imageSize);
			memcpy(data, pixels, static_cast<size_t>(imageSize));
			stagingBufferMemory.unmapMemory();

			stbi_image_free(pixels);

			createImage(device, physical_device, texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
				vk::MemoryPropertyFlagBits::eDeviceLocal,
				texture_image, texture_image_memory_);

			transitionImageLayout(device, command_pool, gragraphics_queue, *texture_image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined,
				vk::ImageLayout::eTransferDstOptimal);
			copyBufferToImage(device, command_pool, gragraphics_queue, stagingBuffer, *texture_image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
			transitionImageLayout(device, command_pool, gragraphics_queue, *texture_image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal,
				vk::ImageLayout::eShaderReadOnlyOptimal);
		}

		void createImage(const LogicalDevice& device, const PhysicalDevice& physical_device, uint32_t width, uint32_t height,
			vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, std::unique_ptr<vk::raii::Image>& image,
			std::unique_ptr<vk::raii::DeviceMemory>& imageMemory)
		{
			vk::ImageCreateInfo imageInfo{};
			imageInfo.imageType = vk::ImageType::e2D;
			imageInfo.extent.width = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = format;
			imageInfo.tiling = tiling;
			imageInfo.initialLayout = vk::ImageLayout::eUndefined;
			imageInfo.usage = usage;
			imageInfo.samples = vk::SampleCountFlagBits::e1;
			imageInfo.sharingMode = vk::SharingMode::eExclusive;

			image = std::make_unique<vk::raii::Image>(device->createImage(imageInfo));

			vk::MemoryRequirements memRequirements = image->getMemoryRequirements();

			vk::MemoryAllocateInfo allocInfo{};
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = physical_device.findMemoryType(memRequirements.memoryTypeBits, properties);

			imageMemory = std::make_unique<vk::raii::DeviceMemory>(device->allocateMemory(allocInfo));

			image->bindMemory(**imageMemory, 0);
		}

		void transitionImageLayout(const LogicalDevice& device, const CommandPool& command_pool, vk::raii::Queue gragraphics_queue,
			vk::raii::Image& image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
			vk::raii::CommandBuffer commandBuffer = command_pool.beginSingleTimeCommands(device);

			vk::ImageMemoryBarrier barrier{};
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = *image;
			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

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
			vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height) {
			vk::raii::CommandBuffer commandBuffer = command_pool.beginSingleTimeCommands(device);

			vk::BufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = vk::Offset3D{ 0, 0, 0 };
			region.imageExtent = vk::Extent3D{
				width,
				height,
				1
			};

			commandBuffer.copyBufferToImage(*buffer, *image, vk::ImageLayout::eTransferDstOptimal, region);

			command_pool.endSingleTimeCommands(gragraphics_queue, commandBuffer);
		}

		void createTextureImageView(const LogicalDevice& device)
		{
			vk::ImageViewCreateInfo viewInfo{};
			viewInfo.image = **texture_image;
			viewInfo.viewType = vk::ImageViewType::e2D;
			viewInfo.format = vk::Format::eR8G8B8A8Srgb;
			viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			image_view_ = std::make_unique<vk::raii::ImageView>(device->createImageView(viewInfo));
		}

		void createTextureSampler(const LogicalDevice& device, const PhysicalDevice& physical_device)
		{
			vk::PhysicalDeviceProperties properties = physical_device->getProperties();

			vk::SamplerCreateInfo samplerInfo{};
			samplerInfo.magFilter = vk::Filter::eLinear;
			samplerInfo.minFilter = vk::Filter::eLinear;
			samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
			samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
			samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
			samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = vk::CompareOp::eAlways;
			samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;

			sampler_ = std::make_unique<vk::raii::Sampler>(device->createSampler(samplerInfo));
		}
	};
}
