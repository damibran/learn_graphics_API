#pragma once
#include <stdexcept>
#include <cstring>
#include <cstdint>

#include "Utils/image_data.h"

#include "Singletons/Singletons.h"

namespace dmbrn
{
	/**
	 * \brief manages texture image data on GPU
	 * TODO need to cache current layout it makes transitioning clearer 
	 */
	class Texture 
	{
	public:
		Texture(const Texture&)=delete;
		Texture& operator=(const Texture&)=delete;

		Texture(Texture&&)=default;
		Texture& operator=(Texture&&)=default;

		Texture() :
			texture_image(nullptr),
			texture_image_memory_(nullptr),
			image_view_(nullptr),
			sampler_(nullptr)
		{
		}

		Texture(vk::Extent2D extent) :
			texture_image(nullptr),
			texture_image_memory_(nullptr),
			image_view_(nullptr),
			sampler_(nullptr)
		{
			createTextureImageWithSize(extent, Singletons::physical_device, Singletons::device,
			                           Singletons::command_pool, Singletons::graphics_queue);
			createTextureImageView(Singletons::device);
			createTextureSampler(Singletons::device, Singletons::physical_device);
		}

		Texture(const image_data& image_data, bool gen_mip_maps = false) :
			texture_image(nullptr),
			texture_image_memory_(nullptr),
			image_view_(nullptr),
			sampler_(nullptr)
		{
			if (gen_mip_maps)
			{
				const uint32_t mipLevels = static_cast<uint32_t>(std::floor(
					std::log2(std::max(image_data.width, image_data.height)))) + 1;
				createTextureImageMipMap(image_data, Singletons::physical_device, Singletons::device,
				                         Singletons::command_pool, Singletons::graphics_queue, mipLevels);
				generateMipmaps(texture_image, image_data.width, image_data.height, mipLevels);
				createTextureImageView(Singletons::device, mipLevels);
				createTextureSampler(Singletons::device, Singletons::physical_device, mipLevels);
			}
			else
			{
				createTextureImage(image_data, Singletons::physical_device, Singletons::device,
				                   Singletons::command_pool, Singletons::graphics_queue);
				createTextureImageView(Singletons::device);
				createTextureSampler(Singletons::device, Singletons::physical_device);
			}
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
		                                const vk::raii::Queue& graphics_queue)
		{
			createImage(extent, vk::Format::eR8G8B8A8Srgb,
			            vk::ImageTiling::eOptimal,
			            vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
			            vk::MemoryPropertyFlagBits::eDeviceLocal,
			            texture_image, texture_image_memory_);

			singleTimeTransitionImageLayout(device, command_pool, graphics_queue, texture_image,
			                                vk::ImageLayout::eUndefined,
			                                vk::ImageLayout::eShaderReadOnlyOptimal);
		}

		void createTextureImage(const image_data& image_data, const PhysicalDevice& physical_device,
		                        const LogicalDevice& device, const CommandPool& command_pool,
		                        vk::raii::Queue graphics_queue)
		{
			const vk::DeviceSize imageSize = image_data.getLength();

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
			memcpy(data, image_data.data.data(), imageSize);
			stagingBufferMemory.unmapMemory();

			createImage({static_cast<uint32_t>(image_data.width), static_cast<uint32_t>(image_data.height)},
			            vk::Format::eR8G8B8A8Srgb,
			            vk::ImageTiling::eOptimal,
			            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			            vk::MemoryPropertyFlagBits::eDeviceLocal,
			            texture_image, texture_image_memory_);

			singleTimeTransitionImageLayout(device, command_pool, graphics_queue, texture_image,
			                                vk::ImageLayout::eUndefined,
			                                vk::ImageLayout::eTransferDstOptimal);
			copyBufferToImage(device, command_pool, graphics_queue, stagingBuffer, texture_image,
			                  static_cast<uint32_t>(image_data.width), static_cast<uint32_t>(image_data.height));
			singleTimeTransitionImageLayout(device, command_pool, graphics_queue, texture_image,
			                                vk::ImageLayout::eTransferDstOptimal,
			                                vk::ImageLayout::eShaderReadOnlyOptimal);
		}

		void createTextureImageMipMap(const image_data& image_data, const PhysicalDevice& physical_device,
		                              const LogicalDevice& device, const CommandPool& command_pool,
		                              vk::raii::Queue graphics_queue, uint32_t mipLevels)
		{
			const vk::DeviceSize imageSize = image_data.getLength();

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
			memcpy(data, image_data.data.data(), imageSize);
			stagingBufferMemory.unmapMemory();

			createImage({static_cast<uint32_t>(image_data.width), static_cast<uint32_t>(image_data.height)},
			            vk::Format::eR8G8B8A8Srgb,
			            vk::ImageTiling::eOptimal,
			            vk::ImageUsageFlagBits::eTransferSrc|vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			            vk::MemoryPropertyFlagBits::eDeviceLocal,
			            texture_image, texture_image_memory_, mipLevels);

			singleTimeTransitionImageLayout(device, command_pool, graphics_queue, texture_image,
			                                vk::ImageLayout::eUndefined,
			                                vk::ImageLayout::eTransferDstOptimal,mipLevels);
			copyBufferToImage(device, command_pool, graphics_queue, stagingBuffer, texture_image,
			                  static_cast<uint32_t>(image_data.width), static_cast<uint32_t>(image_data.height));
			//singleTimeTransitionImageLayout(device, command_pool, graphics_queue, texture_image,
			//                                vk::ImageLayout::eTransferDstOptimal,
			//                                vk::ImageLayout::eShaderReadOnlyOptimal);
			// wiil be completed in genMipMaps
		}

		void createImage(vk::Extent2D extent,
		                 vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
		                 vk::MemoryPropertyFlags properties, vk::raii::Image& image,
		                 vk::raii::DeviceMemory& imageMemory, uint32_t mipLevels_count = 1) const
		{
			const vk::ImageCreateInfo imageInfo
			{
				{}, vk::ImageType::e2D,
				format,
				vk::Extent3D{extent, 1},
				mipLevels_count, 1, vk::SampleCountFlagBits::e1,
				tiling, usage, vk::SharingMode::eExclusive, {},
				{}, vk::ImageLayout::eUndefined
			};

			image = vk::raii::Image{Singletons::device->createImage(imageInfo)};

			const vk::MemoryRequirements memRequirements = image.getMemoryRequirements();

			const vk::MemoryAllocateInfo allocInfo
			{
				memRequirements.size,
				Singletons::physical_device.findMemoryType(memRequirements.memoryTypeBits, properties)
			};

			imageMemory = vk::raii::DeviceMemory{Singletons::device->allocateMemory(allocInfo)};

			image.bindMemory(*imageMemory, 0);
		}

		void singleTimeTransitionImageLayout(const LogicalDevice& device, const CommandPool& command_pool,
		                                     const vk::raii::Queue& gragraphics_queue,
		                                     vk::raii::Image& image, vk::ImageLayout oldLayout,
		                                     vk::ImageLayout newLayout, uint32_t mipLevels=1)
		{
			vk::raii::CommandBuffer commandBuffer = command_pool.beginSingleTimeCommands(device);

			vk::ImageMemoryBarrier barrier
			{
				{}, {}, oldLayout, newLayout,
				VK_QUEUE_FAMILY_IGNORED,VK_QUEUE_FAMILY_IGNORED, *image,
				vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1}
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

		void generateMipmaps(const vk::raii::Image& image, int32_t texWidth, int32_t texHeight,
		                     uint32_t mipLevels)
		{
			// Check if image format supports linear blitting
			const vk::FormatProperties formatProperties = Singletons::physical_device->getFormatProperties(
				vk::Format::eR8G8B8A8Srgb);

			if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
			{
				throw std::runtime_error("texture image format does not support linear blitting!");
			}

			vk::raii::CommandBuffer commandBuffer = Singletons::command_pool.
				beginSingleTimeCommands(Singletons::device);

			vk::ImageMemoryBarrier barrier{};
			barrier.image = *image;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.subresourceRange.levelCount = 1;

			int32_t mipWidth = texWidth;
			int32_t mipHeight = texHeight;

			for (uint32_t i = 1; i < mipLevels; i++)
			{
				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
				barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
				barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
				barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

				commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
				                              vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, {barrier});

				vk::ImageBlit blit{};
				blit.srcOffsets[0] = vk::Offset3D{0, 0, 0};
				blit.srcOffsets[1] = vk::Offset3D{mipWidth, mipHeight, 1};
				blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
				blit.srcSubresource.mipLevel = i - 1;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 1;
				blit.dstOffsets[0] = vk::Offset3D{0, 0, 0};
				blit.dstOffsets[1] = vk::Offset3D{
					mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1
				};
				blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = 1;

				commandBuffer.blitImage(*image, vk::ImageLayout::eTransferSrcOptimal, *image,
				                        vk::ImageLayout::eTransferDstOptimal, {blit}, vk::Filter::eLinear);

				barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
				barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
				barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

				commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
				                              vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, {barrier});

				if (mipWidth > 1) mipWidth /= 2;
				if (mipHeight > 1) mipHeight /= 2;
			}

			barrier.subresourceRange.baseMipLevel = mipLevels - 1;
			barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
			                              vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, {barrier});

			Singletons::command_pool.endSingleTimeCommands(Singletons::graphics_queue, commandBuffer);
		}

		void copyBufferToImage(const LogicalDevice& device, const CommandPool& command_pool,
		                       vk::raii::Queue graphics_queue,
		                       const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height)
		{
			vk::raii::CommandBuffer commandBuffer = command_pool.beginSingleTimeCommands(device);

			const vk::BufferImageCopy region
			{
				0, 0, 0,
				vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1},
				vk::Offset3D{0, 0, 0},
				vk::Extent3D{width, height, 1}
			};

			commandBuffer.copyBufferToImage(*buffer, *image, vk::ImageLayout::eTransferDstOptimal, region);

			command_pool.endSingleTimeCommands(graphics_queue, commandBuffer);
		}

		void createTextureImageView(const LogicalDevice& device, uint32_t mipLevels = 1)
		{
			const vk::ImageViewCreateInfo viewInfo
			{
				{}, *texture_image,
				vk::ImageViewType::e2D,
				vk::Format::eR8G8B8A8Srgb,
				{},
				vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1}
			};

			image_view_ = vk::raii::ImageView{device->createImageView(viewInfo)};
		}

		void createTextureSampler(const LogicalDevice& device, const PhysicalDevice& physical_device,
		                          uint32_t mipLevels = 1)
		{
			const vk::PhysicalDeviceProperties properties = physical_device->getProperties();

			const vk::SamplerCreateInfo samplerInfo
			{
				{}, vk::Filter::eLinear, vk::Filter::eLinear,
				vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eRepeat,
				vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
				{},VK_TRUE, properties.limits.maxSamplerAnisotropy,
				VK_FALSE, vk::CompareOp::eAlways, {}, static_cast<float>(mipLevels), vk::BorderColor::eIntOpaqueBlack,
				VK_FALSE
			};
			sampler_ = vk::raii::Sampler{device->createSampler(samplerInfo)};
		}
	};
}
