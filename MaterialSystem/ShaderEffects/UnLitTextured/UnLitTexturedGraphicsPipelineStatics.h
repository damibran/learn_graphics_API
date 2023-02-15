#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Wrappers/Singletons/LogicalDevice.h"
#include "Wrappers/Singletons/Singletons.h"
#include "EditorUI/Viewport/CameraRenderData.h"
#include "UnLitTexturedGraphicsPipeline.h"
#include "UnLitTexturedRenderData.h"

namespace dmbrn
{
	class UnLitTexturedGraphicsPipelineStatics
	{
	public:
		UnLitTexturedGraphicsPipelineStatics():
			graphics_pipeline_(nullptr), // RAII violation !!!
			render_data_(1)
		{
		}

		void bindPipeline(const vk::raii::CommandBuffer& command_buffer)
		{
			command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
			                            *graphics_pipeline_);
		}

		void bindShaderData(int frame, const vk::raii::CommandBuffer& command_buffer)
		{
			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline_layout_, 1,
			                                  *render_data_[frame], {});
		}

		void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			graphics_pipeline_ = UnLitTexturedGraphicsPipeline::setRenderPass(
				Singletons::device, render_pass, pipeline_layout_);
		}

		void setRenderPass(const vk::raii::RenderPass& render_pass, vk::StencilOpState stencil_op)
		{
			graphics_pipeline_ = UnLitTexturedGraphicsPipeline::setRenderPass(
				Singletons::device, render_pass, pipeline_layout_,
				stencil_op);
		}

		vk::raii::PipelineLayout pipeline_layout_ = createStencilPipelineLayout(Singletons::device);


	private:
		vk::raii::Pipeline graphics_pipeline_;
		UnLitTexturedRenderData render_data_;

		vk::raii::PipelineLayout createStencilPipelineLayout(const LogicalDevice& device)
		{
			std::array<vk::DescriptorSetLayout, 4> descriptor_set_layouts{
				*CameraRenderData::getDescriptorSetLayout(),
				*UnLitTexturedRenderData::getDescriptorSetLayout(),
				*DiffusionDescriptorSets::descriptor_layout_,
				*PerObjectDataBuffer::descriptor_layout_
			};

			const vk::PipelineLayoutCreateInfo pipelineLayoutInfo
			{
				{}, descriptor_set_layouts
			};

			return vk::raii::PipelineLayout{device->createPipelineLayout(pipelineLayoutInfo)};
		}
	};
}
