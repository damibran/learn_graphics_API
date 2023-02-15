#pragma once

#include "OutlineGraphicsPipelineStatics.h"
#include "MaterialSystem/ShaderEffects/ShaderEffect.h"
#include "OutlineShaderEffectRenderData.h"

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
				stencil_graphics_pipeline_.bindPipeline(command_buffer);
				stencil_graphics_pipeline_.bindShaderData(frame, command_buffer);

				auto& [mesh, material, offset] = render_queue.front();

				material->bindMaterialData(frame, command_buffer,
				                           *stencil_graphics_pipeline_.pipeline_layout_);

				mesh->bind(command_buffer);

				per_object_data_buffer.bindDataFor(frame, command_buffer,
				                                   *stencil_graphics_pipeline_.pipeline_layout_,
				                                   offset);

				mesh->drawIndexed(command_buffer);

				outline_graphics_pipeline_statics_.bindPipeline(command_buffer);
				outline_graphics_pipeline_statics_.bindShaderData(frame, command_buffer);

				per_object_data_buffer.bindDataFor(frame, command_buffer,
				                                   *outline_graphics_pipeline_statics_.pipeline_layout_,
				                                   offset);

				mesh->drawIndexed(command_buffer);

				render_queue.pop();
			}
		}

		static void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			const vk::StencilOpState stencil_op
			{
				vk::StencilOp::eReplace, vk::StencilOp::eReplace, vk::StencilOp::eReplace, vk::CompareOp::eAlways, 0xff,
				0xff, 1

			};
			stencil_graphics_pipeline_.setRenderPass(render_pass, stencil_op);

			outline_graphics_pipeline_statics_.setRenderPass(render_pass);
		}

		static inline UnLitTexturedGraphicsPipelineStatics stencil_graphics_pipeline_;
		static inline OutlineGraphicsPipelineStatics outline_graphics_pipeline_statics_{};
	};
}
