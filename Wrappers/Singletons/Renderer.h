#pragma once

#include "Wrappers/Singletons/Singletons.h"
#include "Materials/UnLitTextured/UnLitTexturedDescriptorsStatics.h"
#include "Materials/UnLitTextured/UnlitTexturedMaterial.h"

namespace dmbrn
{
	class Renderer
	{
	public:
		static void Submit(int curentFrame, const vk::raii::CommandBuffer& command_buffer,
		                   const UnlitTexturedMaterial* material, const Mesh& mesh)
		{
			command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
			                            **material->un_lit_descriptors_statics_.graphics_pipeline_);

			command_buffer.bindVertexBuffers(0, {*mesh.vertex_buffer_}, {0});

			command_buffer.bindIndexBuffer(*mesh.index_buffer_, 0, vk::IndexType::eUint16);

			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			                                  *material->un_lit_descriptors_statics_.pipeline_layout_, 0,
			                                  *material->descriptor_sets_[curentFrame], nullptr);

			command_buffer.drawIndexed(mesh.indices_count, 1, 0, 0, 0);
		}

		static void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			UnlitTexturedMaterial::setRenderPass(render_pass);
		}
	};
}
