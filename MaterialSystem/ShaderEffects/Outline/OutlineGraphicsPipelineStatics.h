#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Wrappers/Singletons/LogicalDevice.h"
#include "Wrappers/Singletons/Singletons.h"
#include "EditorUI/Viewport/CameraRenderData.h"
#include "OutlineGraphicsPipeline.h"
#include "MaterialSystem/Materials/Diffusion/DiffusionDescriptorSets.h"
#include "Wrappers/Singletons/PerRenderableData.h"
#include "OutlineShaderEffectRenderData.h"

namespace dmbrn
{
	class OutlineGraphicsPipelineStatics
	{
	public:
		OutlineGraphicsPipelineStatics()
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
			graphics_pipeline_ = OutlineGraphicsPipeline::setRenderPass(
				Singletons::device, render_pass, pipeline_layout_);
		}

		vk::raii::PipelineLayout pipeline_layout_ = createOutlinePipelineLayout(Singletons::device);

	private:
		vk::raii::Pipeline graphics_pipeline_{nullptr}; // RAII violation
		OutlineShaderEffectRenderData render_data_{{255, 255, 0}, 1.01};

		vk::raii::PipelineLayout createOutlinePipelineLayout(const LogicalDevice& device)
		{
			std::array<vk::DescriptorSetLayout, 4> descriptor_set_layouts{
				*CameraRenderData::getDescriptorSetLayout(),
				*OutlineShaderEffectRenderData::getDescriptorSetLayout(),
				*DiffusionDescriptorSets::descriptor_layout_,
				*PerStaticModelData::descriptor_layout_
			};

			const vk::PipelineLayoutCreateInfo pipelineLayoutInfo
			{
				{}, descriptor_set_layouts
			};

			return vk::raii::PipelineLayout{device->createPipelineLayout(pipelineLayoutInfo)};
		}
	};
}
