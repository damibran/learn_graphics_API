#pragma once
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

#include "GLFWwindowWrapper.h"
#include "Instance.h"
#include "Surface.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "DescriptorSetLayout.h"
#include "GraphicsPipeline.h"
#include "CommandPool.h"
#include "Texture.h"
#include "VertexIndexBuffers.h"
#include "UniformBuffers.h"

namespace dmbrn
{
	class DescriptorSets
	{
	public:
		DescriptorSets(const LogicalDevice& device, const DescriptorSetLayout& descriptor_set_layout,
			const UniformBuffers& uniform_buffers, const Texture& texture)
		{
			createDescriptorPool(device);
			createDescriptorSets(device, descriptor_set_layout, uniform_buffers, texture);
		}

		const vk::raii::DescriptorSet& operator[](int index)const
		{
			return descriptor_sets_[index];
		}

	private:
		std::unique_ptr<vk::raii::DescriptorPool> descriptor_pool_;
		std::vector<vk::raii::DescriptorSet> descriptor_sets_;
		void createDescriptorPool(const LogicalDevice& device)
		{
			std::array<vk::DescriptorPoolSize, 2> poolSizes{};
			poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
			poolSizes[0].descriptorCount = static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT);
			poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
			poolSizes[1].descriptorCount = static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT);

			vk::DescriptorPoolCreateInfo poolInfo{};
			poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT);

			descriptor_pool_ = std::make_unique<vk::raii::DescriptorPool>(device->createDescriptorPool(poolInfo));
		}

		void createDescriptorSets(const LogicalDevice& device, const DescriptorSetLayout& descriptor_set_layout,
			const UniformBuffers& uniform_buffers, const Texture& texture)
		{
			std::vector<vk::DescriptorSetLayout> layouts(device.MAX_FRAMES_IN_FLIGHT, **descriptor_set_layout);
			vk::DescriptorSetAllocateInfo allocInfo{};
			allocInfo.descriptorPool = **descriptor_pool_;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT);
			allocInfo.pSetLayouts = layouts.data();

			descriptor_sets_ = device->allocateDescriptorSets(allocInfo);

			for (size_t i = 0; i < device.MAX_FRAMES_IN_FLIGHT; i++) {
				vk::DescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = *uniform_buffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(UniformBuffers::UniformBufferObject);

				vk::DescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = texture.getLayout();
				imageInfo.imageView = *texture.getImageView();
				imageInfo.sampler = *texture.getSampler();

				std::array<vk::WriteDescriptorSet, 2> descriptorWrites{};

				descriptorWrites[0].dstSet = *descriptor_sets_[i];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &bufferInfo;

				descriptorWrites[1].dstSet = *descriptor_sets_[i];
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].pImageInfo = &imageInfo;

				device->updateDescriptorSets(descriptorWrites, nullptr);
			}
		}
	};
}
