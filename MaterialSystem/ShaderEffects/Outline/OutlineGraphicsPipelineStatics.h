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
		OutlineGraphicsPipelineStatics():
			graphics_static_pipeline_(nullptr), // RAII violation !!!
			graphics_skeletal_pipeline_(nullptr)
		{
		}

		void bindStaticPipeline(const vk::raii::CommandBuffer& command_buffer)
		{
			command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
			                            *graphics_static_pipeline_);
		}

		void bindSkeletalPipeline(const vk::raii::CommandBuffer& command_buffer)
		{
			command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
			                            *graphics_skeletal_pipeline_);
		}

		void bindStaticSharedData(int frame, const vk::raii::CommandBuffer& command_buffer)
		{
			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *static_pipeline_layout_, 1,
			                                  *render_data_[frame], {});
		}

		void bindSkeletalSharedData(int frame, const vk::raii::CommandBuffer& command_buffer)
		{
			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *skeletal_pipeline_layout_, 1,
			                                  *render_data_[frame], {});
		}

		void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			graphics_static_pipeline_ = OutlineGraphicsPipeline::createStaticPipeline(
				Singletons::device, render_pass, static_pipeline_layout_);

			graphics_skeletal_pipeline_ = OutlineGraphicsPipeline::createSkeletalPipeline
			(Singletons::device, render_pass, static_pipeline_layout_);
		}

		vk::raii::PipelineLayout static_pipeline_layout_ = createStaticPipelineLayout(Singletons::device);
		vk::raii::PipelineLayout skeletal_pipeline_layout_ = createSkeletalPipelineLayout(Singletons::device);

	private:
		vk::raii::Pipeline graphics_static_pipeline_; // RAII violation
		vk::raii::Pipeline graphics_skeletal_pipeline_;
		OutlineShaderEffectRenderData render_data_{{255, 255, 0}, 1.01f};

		vk::raii::PipelineLayout createStaticPipelineLayout(const LogicalDevice& device)
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

		vk::raii::PipelineLayout createSkeletalPipelineLayout(const LogicalDevice& device)
		{
			std::array<vk::DescriptorSetLayout, 4> descriptor_set_layouts{
				*CameraRenderData::getDescriptorSetLayout(),
				*OutlineShaderEffectRenderData::getDescriptorSetLayout(),
				*DiffusionDescriptorSets::descriptor_layout_,
				*PerSkeletonData::descriptor_layout_
			};

			const vk::PipelineLayoutCreateInfo pipelineLayoutInfo
			{
				{}, descriptor_set_layouts
			};

			return vk::raii::PipelineLayout{device->createPipelineLayout(pipelineLayoutInfo)};
		}
	};
}
