#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Wrappers/Texture.h"

namespace dmbrn
{
	class DiffusionDescriptorSets
	{
	public:
		~DiffusionDescriptorSets() = default;
		DiffusionDescriptorSets(const DiffusionDescriptorSets&) = delete;
		DiffusionDescriptorSets(DiffusionDescriptorSets&&) = default;
		DiffusionDescriptorSets& operator=(DiffusionDescriptorSets&&) = default;

		DiffusionDescriptorSets(const LogicalDevice& device, const Texture& texture, const DiffusionUniformBuffer& uniform_buffer)
		{
			createDescriptorSets(device, texture, uniform_buffer);
		}

		const vk::raii::DescriptorSet& operator[](uint32_t index) const
		{
			return descriptor_sets_[index];
		}

		static vk::raii::DescriptorSetLayout createDescriptorLayout(const LogicalDevice& device)
		{
			const vk::DescriptorSetLayoutBinding samplerLayoutBinding
			{
				0, vk::DescriptorType::eCombinedImageSampler,
				1, vk::ShaderStageFlagBits::eFragment
			};

			const vk::DescriptorSetLayoutBinding baseColorLayoutBinding
			{
				1, vk::DescriptorType::eUniformBuffer,
				1, vk::ShaderStageFlagBits::eFragment
			};

			std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {samplerLayoutBinding, baseColorLayoutBinding};

			const vk::DescriptorSetLayoutCreateInfo layoutInfo
			{
				{},
				bindings
			};

			return vk::raii::DescriptorSetLayout{device->createDescriptorSetLayout(layoutInfo)};
		}

		static inline vk::raii::DescriptorSetLayout descriptor_layout_ = createDescriptorLayout(Singletons::device);

	private:
		std::vector<vk::raii::DescriptorSet> descriptor_sets_;

		void createDescriptorSets(const LogicalDevice& device, const Texture& texture, const DiffusionUniformBuffer& uniform_buffer)
		{
			std::vector<vk::DescriptorSetLayout> layouts(device.MAX_FRAMES_IN_FLIGHT, *descriptor_layout_);

			const vk::DescriptorSetAllocateInfo allocInfo
			{
				**Singletons::descriptor_pool,
				device.MAX_FRAMES_IN_FLIGHT,
				layouts.data()
			};

			descriptor_sets_ = device->allocateDescriptorSets(allocInfo);

			for (uint32_t i = 0; i < device.MAX_FRAMES_IN_FLIGHT; i++)
			{
				const vk::DescriptorImageInfo imageInfo
				{
					*texture.getSampler(), *texture.getImageView(), vk::ImageLayout::eShaderReadOnlyOptimal
				};

				const vk::DescriptorBufferInfo buffer_info
				{
					*uniform_buffer[i],0,sizeof(DiffusionUniformBuffer::UniformBufferObject)
				};

				std::array<vk::WriteDescriptorSet, 2> descriptorWrites{};

				descriptorWrites[0] = vk::WriteDescriptorSet
				{
					*descriptor_sets_[i], 0, 0, vk::DescriptorType::eCombinedImageSampler,
					imageInfo
				};

				descriptorWrites[1] = vk::WriteDescriptorSet{
					*descriptor_sets_[i], 1, 0, vk::DescriptorType::eUniformBuffer,{}, buffer_info
				};

				device->updateDescriptorSets(descriptorWrites, nullptr);
			}
		}
	};
}
