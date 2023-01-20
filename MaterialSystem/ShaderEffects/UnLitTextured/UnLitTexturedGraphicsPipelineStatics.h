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
			pipeline_layout_(createPipelineLayout(Singletons::device))
		{
		}

		void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			graphics_pipeline_.setRenderPass(Singletons::device, render_pass, pipeline_layout_);
		}
		
		vk::raii::PipelineLayout pipeline_layout_;
		UnLitTexturedGraphicsPipeline graphics_pipeline_;

	private:

		vk::raii::PipelineLayout createPipelineLayout(const LogicalDevice& device)
		{
			std::array<vk::DescriptorSetLayout,3> descriptor_set_layouts{*CameraRenderData::getDescriptorSetLayout(), *DiffusionDescriptorSets::descriptor_layout_, *PerObjectDataBuffer::descriptor_layout_};

			const vk::PipelineLayoutCreateInfo pipelineLayoutInfo
			{
				{}, descriptor_set_layouts
			};

			return vk::raii::PipelineLayout{device->createPipelineLayout(pipelineLayoutInfo)};
		}
	};
}
