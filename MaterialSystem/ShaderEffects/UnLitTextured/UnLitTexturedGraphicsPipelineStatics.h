#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Wrappers/Singletons/Singletons.h"
#include "EditorUI/Viewport/CameraRenderData.h"
#include "UnLitTexturedGraphicsPipeline.h"
#include "UnLitTexturedRenderData.h"
#include "Wrappers/Singletons/PerStaticModelData.h"
#include "Wrappers/Singletons/PerSkeletonData.h"

namespace dmbrn
{
	/**
	 * \brief manages objects needed for drawing with UnlitTexturedShaderEffect
	 */
	class UnLitTexturedGraphicsPipelineStatics
	{
	public:
		UnLitTexturedGraphicsPipelineStatics():
			graphics_static_pipeline_(nullptr), // RAII violation !!!
			graphics_skeletal_pipeline_(nullptr),
			render_data_(1)
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

		void bindStaticShaderData(int frame, const vk::raii::CommandBuffer& command_buffer)
		{
			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *static_pipeline_layout_, 1,
			                                  *render_data_[frame], {});
		}

		void bindSkeletalShaderData(int frame, const vk::raii::CommandBuffer& command_buffer)
		{
			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *skeletal_pipeline_layout_, 1,
			                                  *render_data_[frame], {});
		}

		void setRenderPass(const vk::raii::RenderPass& render_pass, bool stencil = false,vk::StencilOpState stencil_op={})
		{
			graphics_static_pipeline_ = UnLitTexturedGraphicsPipeline::createStaticPipeline(
				Singletons::device, render_pass, static_pipeline_layout_,stencil,
				stencil_op);
			graphics_skeletal_pipeline_ = UnLitTexturedGraphicsPipeline::createSkeletalPipeline(Singletons::device, render_pass, skeletal_pipeline_layout_,stencil,
				stencil_op);
		}

		vk::raii::PipelineLayout static_pipeline_layout_ = createStaticPipelineLayout(Singletons::device);
		vk::raii::PipelineLayout skeletal_pipeline_layout_ = createSkeletalPipelineLayout(Singletons::device);

	private:
		vk::raii::Pipeline graphics_static_pipeline_;
		vk::raii::Pipeline graphics_skeletal_pipeline_;
		UnLitTexturedRenderData render_data_;

		vk::raii::PipelineLayout createStaticPipelineLayout(const LogicalDevice& device)
		{
			std::array<vk::DescriptorSetLayout, 4> descriptor_set_layouts{
				*CameraRenderData::getDescriptorSetLayout(),
				*UnLitTexturedRenderData::getDescriptorSetLayout(),
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
				*UnLitTexturedRenderData::getDescriptorSetLayout(),
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
