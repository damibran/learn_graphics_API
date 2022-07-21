#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "LogicalDevice.h"
#include "DescriptorSetLayout.h"
#include "Texture.h"
#include "UniformBuffers.h"

namespace dmbrn
{
	class DescriptorSets
	{
	public:
		DescriptorSets(const LogicalDevice& device, const DescriptorSetLayout& descriptor_set_layout,
			const UniformBuffers& uniform_buffers, const Texture& texture):
			descriptor_pool_(nullptr)
		{
			createDescriptorPool(device);
			createDescriptorSets(device, descriptor_set_layout, uniform_buffers, texture);
		}

		const vk::raii::DescriptorSet& operator[](int index)const
		{
			return descriptor_sets_[index];
		}

	private:
		vk::raii::DescriptorPool descriptor_pool_;
		std::vector<vk::raii::DescriptorSet> descriptor_sets_;

		void createDescriptorPool(const LogicalDevice& device)
		{
			std::array<vk::DescriptorPoolSize, 2> poolSizes{};

			poolSizes[0] = vk::DescriptorPoolSize
			{
				vk::DescriptorType::eUniformBuffer,
				static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT)
			};
			poolSizes[1] = vk::DescriptorPoolSize
			{
				vk::DescriptorType::eCombinedImageSampler,
				static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT)
			};

			vk::DescriptorPoolCreateInfo poolInfo
			{
				vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT),
				static_cast<uint32_t>(poolSizes.size()),
				poolSizes.data()
			};

			descriptor_pool_ = vk::raii::DescriptorPool{device->createDescriptorPool(poolInfo)};
		}

		void createDescriptorSets(const LogicalDevice& device, const DescriptorSetLayout& descriptor_set_layout,
			const UniformBuffers& uniform_buffers, const Texture& texture)
		{
			std::vector<vk::DescriptorSetLayout> layouts(device.MAX_FRAMES_IN_FLIGHT, **descriptor_set_layout);

			const vk::DescriptorSetAllocateInfo allocInfo
			{
				*descriptor_pool_,
				static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT),
				layouts.data()
			};

			descriptor_sets_ = device->allocateDescriptorSets(allocInfo);

			for (size_t i = 0; i < device.MAX_FRAMES_IN_FLIGHT; i++)
			{
				vk::DescriptorBufferInfo bufferInfo
				{
					*uniform_buffers[i],0,sizeof(UniformBuffers::UniformBufferObject)
				};

				vk::DescriptorImageInfo imageInfo
				{
					*texture.getSampler(),*texture.getImageView(),texture.getLayout()
				};

				std::array<vk::WriteDescriptorSet, 2> descriptorWrites{};

				descriptorWrites[0] = vk::WriteDescriptorSet
				{
					*descriptor_sets_[i],0,0,vk::DescriptorType::eUniformBuffer,
					{},bufferInfo
				};

				descriptorWrites[1]=vk::WriteDescriptorSet
				{
					*descriptor_sets_[i], 1,0, vk::DescriptorType::eCombinedImageSampler,
					imageInfo
				};

				device->updateDescriptorSets(descriptorWrites, nullptr);
			}
		}
	};
}
