#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <fstream>
#include <optional>
#include <set>

#include "GLFWwindowWrapper.h"
#include "Instance.h"
#include "Surface.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "SwapChain.h"
#include "RenderPass.h"

namespace dmbrn
{
	class DescriptorSetLayout
	{
	public:
		DescriptorSetLayout(const LogicalDevice& device)
		{
			vk::DescriptorSetLayoutBinding uboLayoutBinding{};
			uboLayoutBinding.binding = 0;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
			uboLayoutBinding.pImmutableSamplers = nullptr;
			uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

			vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
			samplerLayoutBinding.binding = 1;
			samplerLayoutBinding.descriptorCount = 1;
			samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			samplerLayoutBinding.pImmutableSamplers = nullptr;
			samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

			std::array<vk::DescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
			vk::DescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			layoutInfo.pBindings = bindings.data();

			descriptor_set_layout_ = std::make_unique<vk::raii::DescriptorSetLayout>(device->createDescriptorSetLayout(layoutInfo));
		}

		const vk::raii::DescriptorSetLayout& operator*()const
		{
			return *descriptor_set_layout_;
		}

		const vk::raii::DescriptorSetLayout* operator->()const
		{
			return descriptor_set_layout_.get();
		}
	private:
		std::unique_ptr<vk::raii::DescriptorSetLayout> descriptor_set_layout_;
	};
}
