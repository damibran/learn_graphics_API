#pragma once

#include "OutlineGraphicsPipelineStatics.h"
#include "MaterialSystem/ShaderEffects/ShaderEffect.h"

namespace dmbrn
{
	struct UnlitTexturedOutlinedShaderEffect : public ShaderEffect
	{
		//UnlitTexturedOutlinedShaderEffect()=default;

		UnlitTexturedOutlinedShaderEffect(PerStaticModelData& per_object_data_buffer,
		                                  PerSkeletonData& per_skeleton_data):
			per_object_data_buffer_(per_object_data_buffer),
			per_skeleton_data_(per_skeleton_data)
		{
		}

		~UnlitTexturedOutlinedShaderEffect() override = default;

		UnlitTexturedOutlinedShaderEffect(UnlitTexturedOutlinedShaderEffect&& other) = default;
		UnlitTexturedOutlinedShaderEffect& operator=(UnlitTexturedOutlinedShaderEffect&& other) = default;

		UnlitTexturedOutlinedShaderEffect(const UnlitTexturedOutlinedShaderEffect& other) = delete;
		UnlitTexturedOutlinedShaderEffect& operator=(const UnlitTexturedOutlinedShaderEffect& other) = delete;

		void draw(int frame, const vk::raii::CommandBuffer& command_buffer) override
		{
			drawStatic(frame, command_buffer, per_object_data_buffer_);
			drawSkeletal(frame, command_buffer, per_skeleton_data_);
		}

	private:
		PerStaticModelData& per_object_data_buffer_;
		PerSkeletonData& per_skeleton_data_;

		void drawStatic(int frame, const vk::raii::CommandBuffer& command_buffer,
		                const PerStaticModelData& per_object_data_buffer)
		{
			stencil_graphics_pipeline_.bindStaticPipeline(command_buffer);
			stencil_graphics_pipeline_.bindStaticShaderData(frame, command_buffer);
			for (auto& [mesh, offset] : static_render_queue)
			{
				mesh->material_->bindMaterialData(frame, command_buffer, *stencil_graphics_pipeline_.static_pipeline_layout_);

				mesh->bind(command_buffer);

				per_object_data_buffer.bindDataFor(frame, command_buffer,
				                                   *stencil_graphics_pipeline_.static_pipeline_layout_,
				                                   offset);

				mesh->drawIndexed(command_buffer);
			}

			outline_graphics_pipeline_statics_.bindStaticPipeline(command_buffer);
			outline_graphics_pipeline_statics_.bindStaticSharedData(frame, command_buffer);
			for (auto& [mesh, offset] : static_render_queue)
			{
				per_object_data_buffer.bindDataFor(frame, command_buffer,
				                                   *outline_graphics_pipeline_statics_.static_pipeline_layout_,
				                                   offset);

				mesh->drawIndexed(command_buffer);
			}
			static_render_queue.clear();
		}

		void drawSkeletal(int frame, const vk::raii::CommandBuffer& command_buffer,
		                const PerSkeletonData& per_object_data_buffer)
		{
			stencil_graphics_pipeline_.bindSkeletalPipeline(command_buffer);
			stencil_graphics_pipeline_.bindSkeletalShaderData(frame, command_buffer);
			for (auto& [mesh, offset] : skeletal_render_queue)
			{
				mesh->material_->bindMaterialData(frame, command_buffer, *stencil_graphics_pipeline_.skeletal_pipeline_layout_);

				mesh->bind(command_buffer);

				per_object_data_buffer.bindDataFor(frame, command_buffer,
				                                   *stencil_graphics_pipeline_.skeletal_pipeline_layout_,
				                                   offset);

				mesh->drawIndexed(command_buffer);
			}

			outline_graphics_pipeline_statics_.bindSkeletalPipeline(command_buffer);
			outline_graphics_pipeline_statics_.bindSkeletalSharedData(frame, command_buffer);
			for (auto& [mesh, offset] : skeletal_render_queue)
			{
				per_object_data_buffer.bindDataFor(frame, command_buffer,
				                                   *outline_graphics_pipeline_statics_.static_pipeline_layout_,
				                                   offset);

				mesh->drawIndexed(command_buffer);
			}
			skeletal_render_queue.clear();
		}

	public:
		static void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			const vk::StencilOpState stencil_op
			{
				vk::StencilOp::eReplace, vk::StencilOp::eReplace, vk::StencilOp::eReplace, vk::CompareOp::eAlways, 0xff,
				0xff, 1

			};
			stencil_graphics_pipeline_.setRenderPass(render_pass, true, stencil_op);

			outline_graphics_pipeline_statics_.setRenderPass(render_pass);
		}

		static inline UnLitTexturedGraphicsPipelineStatics stencil_graphics_pipeline_;
		static inline OutlineGraphicsPipelineStatics outline_graphics_pipeline_statics_{};
	};
}
