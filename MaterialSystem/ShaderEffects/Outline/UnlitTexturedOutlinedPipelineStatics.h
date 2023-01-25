#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Wrappers/Singletons/LogicalDevice.h"
#include "Wrappers/Singletons/Singletons.h"
#include "EditorUI/Viewport/CameraRenderData.h"
#include "UnlitTexturedOutlinedGraphicsPipeline.h"
#include "OutlineGraphicsPipeline.h"
#include "MaterialSystem/Materials/Diffusion/DiffusionDescriptorSets.h"
#include "Wrappers/Singletons/PerObjectDataBuffer.h"
#include "OutlineShaderEffectRenderData.h"

namespace dmbrn
{
	class UnLitTexturedOutlinedGraphicsPipelineStatics
	{
	public:
		UnLitTexturedOutlinedGraphicsPipelineStatics():
			stencil_pipeline_layout_(createStencilPipelineLayout(Singletons::device)),
			outline_pipeline_layout_(createOutlinePipelineLayout(Singletons::device))
		{
		}

		void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			stencil_graphics_pipeline_.setRenderPass(Singletons::device, render_pass, stencil_pipeline_layout_);
			outline_graphics_pipeline_.setRenderPass(Singletons::device, render_pass, outline_pipeline_layout_);
		}

		vk::raii::PipelineLayout stencil_pipeline_layout_;
		vk::raii::PipelineLayout outline_pipeline_layout_;
		UnLitTexturedOutlinedGraphicsPipeline stencil_graphics_pipeline_;
		OutlineGraphicsPipeline outline_graphics_pipeline_;

	private:
		vk::raii::PipelineLayout createStencilPipelineLayout(const LogicalDevice& device)
		{
			std::array<vk::DescriptorSetLayout, 4> descriptor_set_layouts{
				*CameraRenderData::getDescriptorSetLayout(), *DiffusionDescriptorSets::descriptor_layout_, *empty_descriptor_set_layout_,
				*PerObjectDataBuffer::descriptor_layout_
			};

			const vk::PipelineLayoutCreateInfo pipelineLayoutInfo
			{
				{}, descriptor_set_layouts
			};

			return vk::raii::PipelineLayout{device->createPipelineLayout(pipelineLayoutInfo)};
		}

		vk::raii::PipelineLayout createOutlinePipelineLayout(const LogicalDevice& device)
		{
			std::array<vk::DescriptorSetLayout, 4> descriptor_set_layouts{
				*CameraRenderData::getDescriptorSetLayout(), *DiffusionDescriptorSets::descriptor_layout_,
				*OutlineShaderEffectRenderData::getDescriptorSetLayout(), *PerObjectDataBuffer::descriptor_layout_
			};

			const vk::PipelineLayoutCreateInfo pipelineLayoutInfo
			{
				{}, descriptor_set_layouts
			};

			return vk::raii::PipelineLayout{device->createPipelineLayout(pipelineLayoutInfo)};
		}

		static inline vk::raii::DescriptorSetLayout empty_descriptor_set_layout_{Singletons::device->createDescriptorSetLayout({})};
	};
}
