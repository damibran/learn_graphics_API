#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Wrappers/Singletons/LogicalDevice.h"
#include "Wrappers/Singletons/Singletons.h"
#include "EditorUI/Viewport/CameraRenderData.h"
#include "UnLitTexturedGraphicsPipeline.h"

namespace dmbrn
{
	class UnLitTexturedGraphicsPipelineStatics
	{
	public:

		UnLitTexturedGraphicsPipelineStatics():
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
				vk::ShaderStageFlagBits::eVertex,0,sizeof(glm::mat4)
			};

			std::array<vk::DescriptorSetLayout,2> descriptor_set_layouts{*CameraRenderData::getDescriptorSetLayout(), *UnLitTexturedDescriptorSets::descriptor_layout_};

			const vk::PipelineLayoutCreateInfo pipelineLayoutInfo
			{
				{}, descriptor_set_layouts,push_constant_range
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
