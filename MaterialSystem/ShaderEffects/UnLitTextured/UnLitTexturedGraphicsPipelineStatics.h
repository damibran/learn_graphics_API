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
			stencil_pipeline_layout_(createStencilPipelineLayout(Singletons::device))
		{
		}

		void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			graphics_pipeline_.setRenderPass(Singletons::device, render_pass, stencil_pipeline_layout_);
		}
		
		vk::raii::PipelineLayout stencil_pipeline_layout_;
		UnLitTexturedGraphicsPipeline graphics_pipeline_;

	private:

		vk::raii::PipelineLayout createStencilPipelineLayout(const LogicalDevice& device)
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
