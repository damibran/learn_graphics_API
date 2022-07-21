#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Surface.h"
#include "GLFWwindowWrapper.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "Utils/UtilsFunctions.h"

namespace dmbrn
{
	class DepthBuffer
	{
	public:
		DepthBuffer(const Surface& surface, const GLFWwindowWrapper& window,
			const PhysicalDevice& physical_device, const LogicalDevice& device)
		{
			vk::Format depth_format = utils::findDepthFormat(physical_device);
			vk::Extent2D extent = utils::chooseSwapExtent(PhysicalDevice::querySurfaceCapabilities(*physical_device, surface), window);
			createImage(device, physical_device, extent.width, extent.height, depth_format,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
				vk::MemoryPropertyFlagBits::eDeviceLocal, image_, image_memory_);
			createImageView(device, depth_format);
		}

		void recreate(const Surface& surface, const GLFWwindowWrapper& window,
			const PhysicalDevice& physical_device, const LogicalDevice& device)
		{
			device->waitIdle();
			vk::Format depth_format = utils::findDepthFormat(physical_device);
			vk::Extent2D extent = utils::chooseSwapExtent(PhysicalDevice::querySurfaceCapabilities(*physical_device, surface), window);
			createImage(device, physical_device, extent.width, extent.height, depth_format,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
				vk::MemoryPropertyFlagBits::eDeviceLocal, image_, image_memory_);
			createImageView(device, depth_format);
		}

		const vk::raii::ImageView& operator*() const
		{
			return *image_view_;
		}

	private:
		std::unique_ptr<vk::raii::Image> image_;
		std::unique_ptr<vk::raii::DeviceMemory> image_memory_;
		std::unique_ptr<vk::raii::ImageView> image_view_;

		bool hasStencilComponent(vk::Format format)
		{
			return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
		}

		void createImage(const LogicalDevice& device, const PhysicalDevice& physical_device, uint32_t width, uint32_t height,
			vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, std::unique_ptr<vk::raii::Image>& image,
			std::unique_ptr<vk::raii::DeviceMemory>& imageMemory) const
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

			image = std::make_unique<vk::raii::Image>(device->createImage(imageInfo));

			const vk::MemoryRequirements memRequirements = image->getMemoryRequirements();

			const vk::MemoryAllocateInfo allocInfo
			{
				memRequirements.size,
				physical_device.findMemoryType(memRequirements.memoryTypeBits, properties)
			};

			imageMemory = std::make_unique<vk::raii::DeviceMemory>(device->allocateMemory(allocInfo));

			image->bindMemory(**imageMemory, 0);
		}

		void createImageView(const LogicalDevice& device, vk::Format format)
		{
			const vk::ImageViewCreateInfo viewInfo
			{
				{}, **image_,
				vk::ImageViewType::e2D,
				format,
				{},
				vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth,0,1,0,1}
			};

			image_view_ = std::make_unique<vk::raii::ImageView>(device->createImageView(viewInfo));
		}
	};
}
