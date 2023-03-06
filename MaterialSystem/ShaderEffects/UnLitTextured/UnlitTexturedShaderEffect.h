#pragma once
#include <assimp/material.h>
#include "MaterialSystem/ShaderEffects/ShaderEffect.h"
#include"MaterialSystem/ShaderEffects/UnLitTextured/UnLitTexturedGraphicsPipelineStatics.h"

namespace dmbrn
{
	struct UnlitTexturedShaderEffect : public ShaderEffect
	{
		UnlitTexturedShaderEffect() = default;

		~UnlitTexturedShaderEffect() override = default;

		UnlitTexturedShaderEffect(UnlitTexturedShaderEffect&& other) = default;
		UnlitTexturedShaderEffect& operator=(UnlitTexturedShaderEffect&& other) = default;

		UnlitTexturedShaderEffect(const UnlitTexturedShaderEffect& other) = delete;
		UnlitTexturedShaderEffect& operator=(const UnlitTexturedShaderEffect& other) = delete;

		void draw(int frame, const vk::raii::CommandBuffer& command_buffer,
		          const PerObjectDataBuffer& per_object_data_buffer) override
		{
			un_lit_graphics_pipeline_statics_.bindPipeline(command_buffer);
			un_lit_graphics_pipeline_statics_.bindShaderData(frame, command_buffer);

			for(auto& [mesh, offset]: render_queue){

				mesh->material_->bindMaterialData(frame, command_buffer, *un_lit_graphics_pipeline_statics_.pipeline_layout_);

				mesh->bind(command_buffer);

				per_object_data_buffer.bindDataFor(frame, command_buffer,
				                                   *un_lit_graphics_pipeline_statics_.pipeline_layout_,
				                                   offset);

				mesh->drawIndexed(command_buffer);
			}

			render_queue.clear();
		}

		static void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			un_lit_graphics_pipeline_statics_.setRenderPass(render_pass);
		}

		static inline UnLitTexturedGraphicsPipelineStatics un_lit_graphics_pipeline_statics_{};
	};
}
