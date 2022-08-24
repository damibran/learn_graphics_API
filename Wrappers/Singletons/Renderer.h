#pragma once

#include "Wrappers/Singletons/Singletons.h"
#include "Materials/UnLitTextured/UnLitTexturedDescriptorsStatics.h"
#include "Materials/UnLitTextured/UnlitTexturedMaterial.h"

namespace dmbrn
{
	class Renderer
	{
		friend UnlitTexturedMaterial;

	public:
		static void BeginUnlitTextureMaterial(const vk::raii::CommandBuffer& command_buffer)
		{
			command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
			                            **un_lit_descriptors_statics_.graphics_pipeline_);
		}

		static UnlitTexturedMaterial createUnlitTexturedMaterial()
		{
			return {Singletons::physical_device, Singletons::device, un_lit_descriptors_statics_};
		}

		static void Submit(int curentFrame, const LogicalDevice& device, const vk::raii::CommandBuffer& command_buffer,
		                   const UnlitTexturedMaterial& material, const Mesh& mesh)
		{
			material.descriptor_sets_.updateFrameDescriptorSetTexture(curentFrame, device, mesh.textures_[0]);

			command_buffer.bindVertexBuffers(0, {*mesh.vertex_buffer_}, {0});

			command_buffer.bindIndexBuffer(*mesh.index_buffer_, 0, vk::IndexType::eUint16);

			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			                                  *un_lit_descriptors_statics_.pipeline_layout_, 0,
			                                  *material.descriptor_sets_[curentFrame], nullptr);

			command_buffer.drawIndexed(mesh.indices_count, 1, 0, 0, 0);
		}

	public:
		static void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			un_lit_descriptors_statics_.setRenderPass(render_pass);
		}

	private:
		static inline UnLitTexturedDescriptorsStatics un_lit_descriptors_statics_{};
	};
}
