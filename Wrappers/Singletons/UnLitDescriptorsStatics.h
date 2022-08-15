#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Wrappers/Singletons/LogicalDevice.h"

namespace dmbrn
{
	class UnLitDescriptorsStatics
	{
	public:
		const int MAX_COUNT = 10;

		UnLitDescriptorsStatics(const LogicalDevice& device):
			layout_(createLayout(device)),
			pool_(createDescriptorPool(device))
		{
		}

		vk::raii::DescriptorSetLayout layout_;
		vk::raii::DescriptorPool pool_;
	private:
		vk::raii::DescriptorSetLayout createLayout(const LogicalDevice& device)
		{
			const vk::DescriptorSetLayoutBinding uboLayoutBinding
			{
				0, vk::DescriptorType::eUniformBuffer,
				1, vk::ShaderStageFlagBits::eVertex
			};

			const vk::DescriptorSetLayoutBinding samplerLayoutBinding
			{
				1, vk::DescriptorType::eCombinedImageSampler,
				1, vk::ShaderStageFlagBits::eFragment
			};

			std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

			const vk::DescriptorSetLayoutCreateInfo layoutInfo
			{
				{},
				static_cast<uint32_t>(bindings.size()),
				bindings.data()
			};

			return vk::raii::DescriptorSetLayout{device->createDescriptorSetLayout(layoutInfo)};
		}

		vk::raii::DescriptorPool createDescriptorPool(const LogicalDevice& device) const
		{
			std::array<vk::DescriptorPoolSize, 2> poolSizes{};

			poolSizes[0] = vk::DescriptorPoolSize
			{
				vk::DescriptorType::eUniformBuffer,
				MAX_COUNT * static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT)
			};
			poolSizes[1] = vk::DescriptorPoolSize
			{
				vk::DescriptorType::eCombinedImageSampler,
				MAX_COUNT * static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT)
			};

			vk::DescriptorPoolCreateInfo poolInfo
			{
				vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				MAX_COUNT * static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT),
				static_cast<uint32_t>(poolSizes.size()),
				poolSizes.data()
			};

			return vk::raii::DescriptorPool{device->createDescriptorPool(poolInfo)};
		}
	};
}
