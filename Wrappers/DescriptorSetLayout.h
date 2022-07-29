#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "LogicalDevice.h"

namespace dmbrn
{
	class DescriptorSetLayout
	{
	public:
		DescriptorSetLayout(const LogicalDevice& device):
			descriptor_set_layout_(nullptr)
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

			descriptor_set_layout_ = vk::raii::DescriptorSetLayout{device->createDescriptorSetLayout(layoutInfo)};
		}

		const vk::raii::DescriptorSetLayout& operator*() const
		{
			return descriptor_set_layout_;
		}

		const vk::raii::DescriptorSetLayout* operator->() const
		{
			return &descriptor_set_layout_;
		}

	private:
		vk::raii::DescriptorSetLayout descriptor_set_layout_;
	};
}
