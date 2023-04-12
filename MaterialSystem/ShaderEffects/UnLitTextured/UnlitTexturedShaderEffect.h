#pragma once

#include "MaterialSystem/ShaderEffects/ShaderEffect.h"
#include"MaterialSystem/ShaderEffects/UnLitTextured/UnLitTexturedGraphicsPipelineStatics.h"
#include "Wrappers/Singletons/PerSkeletonData.h"

namespace dmbrn
{
	struct UnlitTexturedShaderEffect : public ShaderEffect
	{
		//UnlitTexturedShaderEffect()=default;

		UnlitTexturedShaderEffect(PerRenderableData& per_object_data_buffer,
		                                  PerSkeletonData& per_skeleton_data):
			per_object_data_buffer_(per_object_data_buffer),
			per_skeleton_data_(per_skeleton_data)
		{
		}

		~UnlitTexturedShaderEffect() override = default;

		UnlitTexturedShaderEffect(UnlitTexturedShaderEffect&& other) = default;
		UnlitTexturedShaderEffect& operator=(UnlitTexturedShaderEffect&& other) = default;

		UnlitTexturedShaderEffect(const UnlitTexturedShaderEffect& other) = delete;
		UnlitTexturedShaderEffect& operator=(const UnlitTexturedShaderEffect& other) = delete;

		void draw(int frame, const vk::raii::CommandBuffer& command_buffer) override
		{
			drawStatic(frame, command_buffer, per_object_data_buffer_);
			//drawSkeletal(frame, command_buffer, per_skeleton_data_);
		}

	private:

		PerRenderableData& per_object_data_buffer_;
		PerSkeletonData& per_skeleton_data_;

		void drawStatic(int frame, const vk::raii::CommandBuffer& command_buffer,
		                const PerRenderableData& per_object_data_buffer) 		{
			un_lit_graphics_pipeline_statics_.bindPipeline(command_buffer);
			un_lit_graphics_pipeline_statics_.bindShaderData(frame, command_buffer);

			for(auto& [mesh, offset]: static_render_queue){

				mesh->material_->bindMaterialData(frame, command_buffer, *un_lit_graphics_pipeline_statics_.pipeline_layout_);

				mesh->bind(command_buffer);

				per_object_data_buffer.bindDataFor(frame, command_buffer,
				                                   *un_lit_graphics_pipeline_statics_.pipeline_layout_,
				                                   offset);

				mesh->drawIndexed(command_buffer);
			}

			static_render_queue.clear();
		}

		void drawSkeletal(int frame, const vk::raii::CommandBuffer& command_buffer,
		                const PerSkeletonData& per_object_data_buffer) 		{
			un_lit_graphics_pipeline_statics_.bindPipeline(command_buffer);
			un_lit_graphics_pipeline_statics_.bindShaderData(frame, command_buffer);

			for(auto& [mesh, offset]: skeletal_render_queue){

				mesh->material_->bindMaterialData(frame, command_buffer, *un_lit_graphics_pipeline_statics_.pipeline_layout_);

				mesh->bind(command_buffer);

				per_object_data_buffer.bindDataFor(frame, command_buffer,
				                                   *un_lit_graphics_pipeline_statics_.pipeline_layout_,
				                                   offset);

				mesh->drawIndexed(command_buffer);
			}

			static_render_queue.clear();
		}

	public:

		static void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			un_lit_graphics_pipeline_statics_.setRenderPass(render_pass);
		}

		static inline UnLitTexturedGraphicsPipelineStatics un_lit_graphics_pipeline_statics_{};
	};
}
