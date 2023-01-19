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

		void draw(int frame, const vk::raii::CommandBuffer& command_buffer) override
		{
			command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
				**un_lit_graphics_pipeline_statics_.graphics_pipeline_);

			while (!render_queue.empty())
			{
				auto [mesh, material, matrix] = render_queue.front();
				render_queue.pop();

				mesh->bind(command_buffer);
				material->bindMaterialData(frame, command_buffer, *un_lit_graphics_pipeline_statics_.pipeline_layout_);

				std::array<glm::mat4, 1> arr{
				{matrix}
				};

				command_buffer.pushConstants<glm::mat4>(
					*un_lit_graphics_pipeline_statics_.pipeline_layout_,
					vk::ShaderStageFlagBits::eVertex, 0, arr);\

				command_buffer.drawIndexed(mesh->indices_count, 1, 0, 0, 0);
			}
		}

		static void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			un_lit_graphics_pipeline_statics_.setRenderPass(render_pass);
		}

		static inline UnLitTexturedGraphicsPipelineStatics un_lit_graphics_pipeline_statics_{};
	};
}
