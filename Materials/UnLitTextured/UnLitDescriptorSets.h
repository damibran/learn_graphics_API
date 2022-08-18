#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "UnLitDescriptorsStatics.h"
#include "Wrappers/Texture.h"
#include "Wrappers/UniformBuffers.h"

namespace dmbrn
{
	class UnLitDescriptorSets
	{
	public:

		UnLitDescriptorSets(UnLitDescriptorSets&&)=default;
		UnLitDescriptorSets& operator=(UnLitDescriptorSets&&)=default;

		UnLitDescriptorSets(const LogicalDevice& device, const UnLitDescriptorsStatics& statics, 
		               const UniformBuffers& uniform_buffers)
		{
			createDescriptorSets(device, statics, uniform_buffers);
		}

		void updateFrameDescriptorSetTexture(uint32_t frame, const LogicalDevice& device, const Texture& texture) const
		{
			vk::DescriptorImageInfo imageInfo
			{
				*texture.getSampler(), *texture.getImageView(), vk::ImageLayout::eShaderReadOnlyOptimal
			};

			vk::WriteDescriptorSet descriptor_write
			{
				*descriptor_sets_[frame], 1, 0, vk::DescriptorType::eCombinedImageSampler,
				imageInfo
			};

			device->updateDescriptorSets(descriptor_write, nullptr);
		}

		const vk::raii::DescriptorSet& operator[](uint32_t index) const
		{
			return descriptor_sets_[index];
		}

	private:
		std::vector<vk::raii::DescriptorSet> descriptor_sets_;
		
		void createDescriptorSets(const LogicalDevice& device, const UnLitDescriptorsStatics& statics,
		                          const UniformBuffers& uniform_buffers)
		{
			std::vector<vk::DescriptorSetLayout> layouts(device.MAX_FRAMES_IN_FLIGHT, *statics.descriptor_layout_);

			const vk::DescriptorSetAllocateInfo allocInfo
			{
				*statics.pool_,
				static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT),
				layouts.data()
			};

			descriptor_sets_ = device->allocateDescriptorSets(allocInfo);

			for (size_t i = 0; i < device.MAX_FRAMES_IN_FLIGHT; i++)
			{
				vk::DescriptorBufferInfo bufferInfo
				{
					*uniform_buffers[i], 0, sizeof(UniformBuffers::UniformBufferObject)
				};

				//vk::DescriptorImageInfo imageInfo
				//{
				//	*texture.getSampler(),*texture.getImageView(),texture.getLayout()
				//};

				std::array<vk::WriteDescriptorSet, 1> descriptorWrites{};

				descriptorWrites[0] = vk::WriteDescriptorSet
				{
					*descriptor_sets_[i], 0, 0, vk::DescriptorType::eUniformBuffer,
					{}, bufferInfo
				};

				//descriptorWrites[1]=vk::WriteDescriptorSet
				//{
				//	*descriptor_sets_[i], 1,0, vk::DescriptorType::eCombinedImageSampler,
				//	imageInfo
				//};

				device->updateDescriptorSets(descriptorWrites, nullptr);
			}
		}
	};
}
