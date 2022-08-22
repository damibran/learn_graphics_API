#pragma once

#include "Wrappers/Singletons/Singletons.h"
#include "Materials/UnLitTextured/UnLitDescriptorsStatics.h"
#include "Materials/UnLitTextured/UnlitTextureMaterial.h"

namespace dmbrn
{
	class Renderer
	{
		friend UnlitTextureMaterial;

	public:
		static void BeginUnlitTextureMaterial(const vk::raii::CommandBuffer& command_buffer)
		{
			command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
			                            **un_lit_descriptors_statics_.graphics_pipeline_);
		}

		static UnlitTextureMaterial createUnlitTexturedMaterial()
		{
			return {Singletons::physical_device, Singletons::device, un_lit_descriptors_statics_};
		}

		static void Submit(int curentFrame, const LogicalDevice& device, const vk::raii::CommandBuffer& command_buffer,
		                   const UnlitTextureMaterial& material, const Mesh& mesh)
		{
			material.descriptor_sets_.updateFrameDescriptorSetTexture(curentFrame, device, mesh.textures_[0]);

			command_buffer.bindVertexBuffers(0, {*mesh.vertex_buffer_}, {0});

			command_buffer.bindIndexBuffer(*mesh.index_buffer_, 0, vk::IndexType::eUint16);

			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			                                  *un_lit_descriptors_statics_.pipeline_layout_, 0,
			                                  *material.descriptor_sets_[curentFrame], nullptr);

			command_buffer.drawIndexed(static_cast<uint32_t>(mesh.indices_.size()), 1, 0, 0, 0);
		}

	public:
		static void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			un_lit_descriptors_statics_.setRenderPass(render_pass);
		}

	private:
		static inline UnLitDescriptorsStatics un_lit_descriptors_statics_{};
	};
}
