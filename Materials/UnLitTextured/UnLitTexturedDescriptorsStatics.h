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

		UnLitTexturedDescriptorsStatics():
			pipeline_layout_(createPipelineLayoutPushConst(Singletons::device))
		{
		}

		void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			graphics_pipeline_.setRenderPass(Singletons::device, render_pass, pipeline_layout_);
		}
		
		vk::raii::PipelineLayout pipeline_layout_;
		UnLitTexturedGraphicsPipeline graphics_pipeline_;

	private:

		vk::raii::PipelineLayout createPipelineLayoutPushConst(const LogicalDevice& device)
		{
			const vk::PushConstantRange push_constant_range
			{
				vk::ShaderStageFlagBits::eVertex,0,sizeof(UniformBuffers::UniformBufferObject)
			};

			const vk::PipelineLayoutCreateInfo pipelineLayoutInfo
			{
				{}, *UnLitTexturedDescriptorSets::descriptor_layout_,push_constant_range
			};

			return vk::raii::PipelineLayout{device->createPipelineLayout(pipelineLayoutInfo)};
		}

		vk::raii::PipelineLayout createPipelineLayout(const LogicalDevice& device)
		{
			const vk::PipelineLayoutCreateInfo pipelineLayoutInfo
			{
				{}, *UnLitTexturedDescriptorSets::descriptor_layout_
			};

			return vk::raii::PipelineLayout{device->createPipelineLayout(pipelineLayoutInfo)};
		}
	};
}
