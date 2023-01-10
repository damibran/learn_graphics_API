#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Wrappers/Texture.h"
#include "Wrappers/UniformBuffers.h"

namespace dmbrn
{
	class UnLitTexturedDescriptorSets
	{
	public:
		static const int MAX_COUNT = 10;

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
				static_cast<uint32_t>(bindings.size()),
				bindings.data()
			};

			return vk::raii::DescriptorSetLayout{device->createDescriptorSetLayout(layoutInfo)};
		}

		static vk::raii::DescriptorSetLayout createDescriptorLayout(const LogicalDevice& device)
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
				*pool_,
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

		static vk::raii::DescriptorPool createDescriptorPoolPushConst(const LogicalDevice& device)
		{
			std::array<vk::DescriptorPoolSize, 1> poolSizes{};

			poolSizes[0] = vk::DescriptorPoolSize
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

		static vk::raii::DescriptorPool createDescriptorPool(const LogicalDevice& device)
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

		static inline vk::raii::DescriptorPool pool_{createDescriptorPoolPushConst(Singletons::device)};
	};
}
