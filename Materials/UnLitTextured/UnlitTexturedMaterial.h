#pragma once
#include <assimp/material.h>
#include "UnLitTexturedDescriptorSets.h"
#include "Materials/Material.h"
#include "Materials/UnLitTextured/UnLitTexturedGraphicsPipelineStatics.h"

namespace dmbrn
{
	struct UnlitTexturedMaterial : Material
	{
		~UnlitTexturedMaterial() override = default;

		UnlitTexturedMaterial(UnlitTexturedMaterial&& other) = default;
		UnlitTexturedMaterial& operator=(UnlitTexturedMaterial&& other) = default;

		UnlitTexturedMaterial(const UnlitTexturedMaterial& other) = delete;
		UnlitTexturedMaterial& operator=(const UnlitTexturedMaterial& other) = delete;

		static UnlitTexturedMaterial* GetMaterialPtr(const std::string& dir, const std::string& model_name,
		                                             const aiMaterial* ai_material)
		{
			std::string material_name = model_name + "." + std::string{ai_material->GetName().data};

			auto it = material_registry.find(material_name);
			if (it == material_registry.end())
				it = material_registry.emplace(material_name, UnlitTexturedMaterial{dir, ai_material}).first;

			return &it->second;
		}

		void draw(const vk::raii::Buffer& vertex_buffer_, const vk::raii::Buffer& index_buffer_, uint32_t indices_count,
		          int frame, const vk::raii::CommandBuffer& command_buffer, const glm::mat4& modelMat,
		          const glm::mat4& view,
		          const glm::mat4& proj) const override
		{
			std::array<CameraUniformBuffer::UniformBufferObject, 1> arr{
				{modelMat, view, proj}
			};

			command_buffer.pushConstants<CameraUniformBuffer::UniformBufferObject>(
				*un_lit_graphics_pipeline_statics_.pipeline_layout_,
				vk::ShaderStageFlagBits::eVertex, 0, arr);

			command_buffer.bindVertexBuffers(0, *vertex_buffer_, {0});

			command_buffer.bindIndexBuffer(*index_buffer_, 0, vk::IndexType::eUint16);

			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			                                  *un_lit_graphics_pipeline_statics_.pipeline_layout_, 0,
			                                  *descriptor_sets_[frame], nullptr);

			command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
			                            **un_lit_graphics_pipeline_statics_.graphics_pipeline_);

			command_buffer.drawIndexed(indices_count, 1, 0, 0, 0);
		}

		static void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			un_lit_graphics_pipeline_statics_.setRenderPass(render_pass);
		}

		Texture diffuse;
		UnLitTexturedDescriptorSets descriptor_sets_;
		static inline UnLitTexturedGraphicsPipelineStatics un_lit_graphics_pipeline_statics_{};
		static inline std::unordered_map<std::string, UnlitTexturedMaterial> material_registry;

	private:
		// checks all material textures of a given type and loads the textures if they're not loaded yet.
		// the required info is returned as a Texture struct.
		//std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& directory)
		//{
		//	std::vector<Texture> textures;
		//	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		//	{
		//		aiString str;
		//		mat->GetTexture(type, i, &str);
		//		std::string filename = directory + '\\' + std::string(str.C_Str());
		//
		//		textures.emplace_back(Texture{filename});
		//	}
		//	return textures;
		//}

		UnlitTexturedMaterial(const std::string& dir,
		                      const aiMaterial* ai_material):
			diffuse(getDeffuseTexturePath(ai_material, aiTextureType_DIFFUSE, dir)),
			descriptor_sets_(Singletons::device, diffuse)
		{
		}

		std::string getDeffuseTexturePath(const aiMaterial* mat, aiTextureType type, const std::string& directory)
		{
			aiString str;
			mat->GetTexture(type, 0, &str);
			std::string filename = directory + '\\' + std::string(str.C_Str());
			return filename;
		}
	};
}
