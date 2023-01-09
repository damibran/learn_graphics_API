#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Wrappers/UniformBuffers.h"
#include "Wrappers/Singletons/LogicalDevice.h"
#include "Wrappers/Singletons/Singletons.h"
#include "UnLitTexturedGraphicsPipeline.h"

namespace dmbrn
{
	class UnLitTexturedDescriptorsStatics
	{
	public:
		const int MAX_COUNT = 10;

		UnLitTexturedDescriptorsStatics():
			descriptor_layout_(createDescriptorLayoutPushConst(Singletons::device)),
			pipeline_layout_(createPipelineLayoutPushConst(Singletons::device)),
			pool_(createDescriptorPoolPushConst(Singletons::device))
		{
		}

		void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			graphics_pipeline_.setRenderPass(Singletons::device, render_pass, pipeline_layout_);
		}
		
		vk::raii::DescriptorSetLayout descriptor_layout_;
		vk::raii::PipelineLayout pipeline_layout_;
		vk::raii::DescriptorPool pool_;
		UnLitTexturedGraphicsPipeline graphics_pipeline_;

	private:
		vk::raii::DescriptorSetLayout createDescriptorLayoutPushConst(const LogicalDevice& device)
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

		vk::raii::DescriptorSetLayout createDescriptorLayout(const LogicalDevice& device)
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

		vk::raii::PipelineLayout createPipelineLayoutPushConst(const LogicalDevice& device)
		{
			const vk::PushConstantRange push_constant_range
			{
				vk::ShaderStageFlagBits::eVertex,0,sizeof(UniformBuffers::UniformBufferObject)
			};

			const vk::PipelineLayoutCreateInfo pipelineLayoutInfo
			{
				{}, *descriptor_layout_,push_constant_range
			};

			return vk::raii::PipelineLayout{device->createPipelineLayout(pipelineLayoutInfo)};
		}

		vk::raii::PipelineLayout createPipelineLayout(const LogicalDevice& device)
		{
			const vk::PipelineLayoutCreateInfo pipelineLayoutInfo
			{
				{}, *descriptor_layout_
			};

			return vk::raii::PipelineLayout{device->createPipelineLayout(pipelineLayoutInfo)};
		}

		vk::raii::DescriptorPool createDescriptorPoolPushConst(const LogicalDevice& device) const
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
