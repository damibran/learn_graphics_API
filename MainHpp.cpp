#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <fstream>
#include <optional>
#include <set>

#include <glm/glm.hpp>
#include <stb_image.h>

#include "Wrappers/GLFWwindowWrapper.h"
#include "Wrappers/Instance.h"
#include "Wrappers/Surface.h"
#include "Wrappers/PhysicalDevice.h"
#include "Wrappers/LogicalDevice.h"
#include "Wrappers/SwapChain.h"
#include "Wrappers/RenderPass.h"
#include "Wrappers/DescriptorSetLayout.h"
#include "Wrappers/GraphicsPipeline.h"
#include "Wrappers/FrameBuffers.h"
#include "Wrappers/CommandPool.h"

namespace dmbrn
{
	class HelloTriangleApplication
	{
	public:

		HelloTriangleApplication(uint32_t width, uint32_t height) :
			window_(width, height),
			instance_(context_),
			surface_(instance_, window_),
			physical_device_(instance_, surface_),
			device_(physical_device_, surface_),
			gragraphics_queue_(device_->getQueue(physical_device_.getQueueFamilyIndices().graphicsFamily.value(), 0)),
			present_queue_(device_->getQueue(physical_device_.getQueueFamilyIndices().presentFamily.value(), 0)),
			swap_chain_(physical_device_, device_, surface_, window_),
			render_pass_(device_, swap_chain_),
			descriptor_set_layout_(device_),
			graphics_pipeline_(device_, render_pass_, descriptor_set_layout_),
			frame_buffers_(device_, swap_chain_, render_pass_),
			command_pool_(physical_device_, device_)
		{
			//createTextureImage();
			//createTextureImageView();
			//createTextureSampler();
			//createVertexBuffer();
			//createIndexBuffer();
			//createUniformBuffers();
			//createDescriptorPool();
			//createDescriptorSets();
			//createCommandBuffers();
			//createSyncObjects();
		}

		void run() {

		}

	private:
		GLFWwindowWrapper window_;
		vk::raii::Context context_;
		Instance instance_;
		Surface surface_;
		PhysicalDevice physical_device_;
		LogicalDevice device_;
		vk::raii::Queue gragraphics_queue_;
		vk::raii::Queue present_queue_;
		SwapChain swap_chain_;
		RenderPass render_pass_;
		DescriptorSetLayout descriptor_set_layout_;
		GraphicsPipeline graphics_pipeline_;
		FrameBuffers frame_buffers_;
		CommandPool command_pool_;

		void createTextureImage()
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

			vk::raii::Buffer stagingBuffer = device_->createBuffer(bufferInfo);

			vk::MemoryRequirements memRequirements = stagingBuffer.getMemoryRequirements();

			vk::MemoryAllocateInfo allocInfo{};
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

			vk::raii::DeviceMemory stagingBufferMemory = device_->allocateMemory(allocInfo);

			stagingBuffer.bindMemory(*stagingBufferMemory, 0);

			void* data;
			data = stagingBufferMemory.mapMemory(0, imageSize);
			memcpy(data, pixels, static_cast<size_t>(imageSize));
			stagingBufferMemory.unmapMemory();

			stbi_image_free(pixels);

			createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

			transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
			transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = format;
			imageInfo.tiling = tiling;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = usage;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image!");
			}

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(device, image, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

			if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate image memory!");
			}

			vkBindImageMemory(device, image, imageMemory, 0);
		}

		void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory)
		{

		}

		uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
		{
			vk::PhysicalDeviceMemoryProperties memProperties = physical_device_->getMemoryProperties();


			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
				if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
					return i;
				}
			}

			throw std::runtime_error("failed to find suitable memory type!");
		}
	};
}

int main()
{
	try {
		dmbrn::HelloTriangleApplication app(800, 600);
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}