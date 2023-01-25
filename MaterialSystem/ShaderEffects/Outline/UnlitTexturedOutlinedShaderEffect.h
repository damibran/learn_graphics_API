#pragma once
#include <assimp/material.h>
#include "MaterialSystem/ShaderEffects/ShaderEffect.h"
#include"MaterialSystem/ShaderEffects/Outline/UnlitTexturedOutlinedPipelineStatics.h"

namespace dmbrn
{
	struct UnlitTexturedOutlinedShaderEffect : public ShaderEffect
	{
		UnlitTexturedOutlinedShaderEffect() = default;

		~UnlitTexturedOutlinedShaderEffect() override = default;

		UnlitTexturedOutlinedShaderEffect(UnlitTexturedOutlinedShaderEffect&& other) = default;
		UnlitTexturedOutlinedShaderEffect& operator=(UnlitTexturedOutlinedShaderEffect&& other) = default;

		UnlitTexturedOutlinedShaderEffect(const UnlitTexturedOutlinedShaderEffect& other) = delete;
		UnlitTexturedOutlinedShaderEffect& operator=(const UnlitTexturedOutlinedShaderEffect& other) = delete;

		void draw(int frame, const vk::raii::CommandBuffer& command_buffer,
		          const PerObjectDataBuffer& per_object_data_buffer) override
		{
			while (!render_queue.empty())
			{
				command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
				                            **outline_graphics_pipeline_statics_.stencil_graphics_pipeline_);

				auto& [mesh, material, offset] = render_queue.front();

				material->bindMaterialData(frame, command_buffer, *outline_graphics_pipeline_statics_.stencil_pipeline_layout_);

				mesh->bind(command_buffer);

				per_object_data_buffer.bindDataFor(frame, command_buffer,
				                                   *outline_graphics_pipeline_statics_.stencil_pipeline_layout_,
				                                   offset);

				command_buffer.drawIndexed(mesh->indices_count, 1, 0, 0, 0);

				command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
				                            **outline_graphics_pipeline_statics_.outline_graphics_pipeline_);

				command_buffer.drawIndexed(mesh->indices_count, 1, 0, 0, 0);

				render_queue.pop();
			}
		}

		static void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			outline_graphics_pipeline_statics_.setRenderPass(render_pass);
		}

		static inline UnLitTexturedOutlinedGraphicsPipelineStatics outline_graphics_pipeline_statics_{};
	};
}
