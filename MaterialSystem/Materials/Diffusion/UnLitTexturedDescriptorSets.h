#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Wrappers/Texture.h"

namespace dmbrn
{
	class UnLitTexturedDescriptorSets
	{
	public:

		~UnLitTexturedDescriptorSets() = default;
		UnLitTexturedDescriptorSets(const UnLitTexturedDescriptorSets&) = delete;
		UnLitTexturedDescriptorSets(UnLitTexturedDescriptorSets&&) = default;
		UnLitTexturedDescriptorSets& operator=(UnLitTexturedDescriptorSets&&) = default;

		UnLitTexturedDescriptorSets(const LogicalDevice& device,
		                            const Texture& texture)
		{
			createDescriptorSets(device, texture);
		}

		const vk::raii::DescriptorSet& operator[](uint32_t index) const
		{
			return descriptor_sets_[index];
		}

		static vk::raii::DescriptorSetLayout createDescriptorLayoutPushConst(const LogicalDevice& device)
		{
			const vk::DescriptorSetLayoutBinding samplerLayoutBinding
			{
				0, vk::DescriptorType::eCombinedImageSampler,
				1, vk::ShaderStageFlagBits::eFragment
			};

			std::array<vk::DescriptorSetLayoutBinding, 1> bindings = {samplerLayoutBinding};

			const vk::DescriptorSetLayoutCreateInfo layoutInfo
			{
				{},
				bindings
			};

			return vk::raii::DescriptorSetLayout{device->createDescriptorSetLayout(layoutInfo)};
		}

		static inline vk::raii::DescriptorSetLayout descriptor_layout_{
			createDescriptorLayoutPushConst(Singletons::device)
		};

	private:
		std::vector<vk::raii::DescriptorSet> descriptor_sets_;

		void createDescriptorSets(const LogicalDevice& device, const Texture& texture)
		{
			std::vector<vk::DescriptorSetLayout> layouts(device.MAX_FRAMES_IN_FLIGHT, *descriptor_layout_);

			const vk::DescriptorSetAllocateInfo allocInfo
			{
				**Singletons::descriptor_pool,
				static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT),
				layouts.data()
			};

			descriptor_sets_ = device->allocateDescriptorSets(allocInfo);

			for (uint32_t i = 0; i < device.MAX_FRAMES_IN_FLIGHT; i++)
			{
				vk::DescriptorImageInfo imageInfo
				{
					*texture.getSampler(), *texture.getImageView(), vk::ImageLayout::eShaderReadOnlyOptimal
				};

				std::array<vk::WriteDescriptorSet, 1> descriptorWrites{};

				descriptorWrites[0] = vk::WriteDescriptorSet
				{
					*descriptor_sets_[i], 0, 0, vk::DescriptorType::eCombinedImageSampler,
					imageInfo
				};

				device->updateDescriptorSets(descriptorWrites, nullptr);
			}
		}
	};
}
