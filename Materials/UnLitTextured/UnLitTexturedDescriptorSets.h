#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "UnLitTexturedDescriptorsStatics.h"
#include "Wrappers/Texture.h"
#include "Wrappers/UniformBuffers.h"

namespace dmbrn
{
	class UnLitTexturedDescriptorSets
	{
	public:
		~UnLitTexturedDescriptorSets()=default;
		UnLitTexturedDescriptorSets(const UnLitTexturedDescriptorSets&)=delete;
		UnLitTexturedDescriptorSets(UnLitTexturedDescriptorSets&&) = default;
		UnLitTexturedDescriptorSets& operator=(UnLitTexturedDescriptorSets&&) = default;

		UnLitTexturedDescriptorSets(const LogicalDevice& device, const UnLitTexturedDescriptorsStatics& statics, const Texture& texture)
		{
			createDescriptorSets(device, statics, texture);
		}

		const vk::raii::DescriptorSet& operator[](uint32_t index) const
		{
			return descriptor_sets_[index];
		}

	private:
		std::vector<vk::raii::DescriptorSet> descriptor_sets_;

		void createDescriptorSets(const LogicalDevice& device, const UnLitTexturedDescriptorsStatics& statics, const Texture& texture)
		{
			std::vector<vk::DescriptorSetLayout> layouts(device.MAX_FRAMES_IN_FLIGHT, *statics.descriptor_layout_);

			const vk::DescriptorSetAllocateInfo allocInfo
			{
				*statics.pool_,
				static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT),
				layouts.data()
			};

			descriptor_sets_ = device->allocateDescriptorSets(allocInfo);

			for (uint32_t i = 0; i < device.MAX_FRAMES_IN_FLIGHT; i++)
			{
				//vk::DescriptorBufferInfo bufferInfo
				//{
				//	*uniform_buffers[i], 0, sizeof(UniformBuffers::UniformBufferObject)
				//};

				vk::DescriptorImageInfo imageInfo
				{
					*texture.getSampler(), *texture.getImageView(), vk::ImageLayout::eShaderReadOnlyOptimal
				};

				std::array<vk::WriteDescriptorSet, 1> descriptorWrites{};

				//descriptorWrites[0] = vk::WriteDescriptorSet
				//{
				//	*descriptor_sets_[i], 0, 0, vk::DescriptorType::eUniformBuffer,
				//	{}, bufferInfo
				//};

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
