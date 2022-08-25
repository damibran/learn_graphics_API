#pragma once
#include <assimp/material.h>
#include "UnLitTexturedDescriptorSets.h"
#include "Materials/UnLitTextured/UnLitTexturedDescriptorsStatics.h"
#include "Wrappers/UniformBuffers.h"

namespace dmbrn
{
	struct UnlitTexturedMaterial
	{
		inline static std::unordered_map<std::string, UnlitTexturedMaterial> material_registry;

		UnlitTexturedMaterial(UnlitTexturedMaterial&& other) = default;
		UnlitTexturedMaterial& operator=(UnlitTexturedMaterial&& other) = default;

		UnlitTexturedMaterial(const UnlitTexturedMaterial& other) = delete;
		UnlitTexturedMaterial& operator=(const UnlitTexturedMaterial& other) = delete;

		UnlitTexturedMaterial(const std::string& dir,
		                      const aiMaterial* ai_material):
			diffuse(getDeffuseTexturePath(ai_material, aiTextureType_DIFFUSE, dir)),
			uniform_buffers_(Singletons::physical_device, Singletons::device),
			descriptor_sets_(Singletons::device, un_lit_descriptors_statics_, uniform_buffers_,diffuse)
		{
		}

		static UnlitTexturedMaterial* GetMaterialPtr(const std::string& dir, const std::string& model_name,
		                                             const aiMaterial* ai_material)
		{
			std::string material_name = model_name + "." + std::string{ai_material->GetName().data};

			return &(*material_registry.emplace(std::piecewise_construct,
			                                    std::forward_as_tuple(material_name),
			                                    std::forward_as_tuple(dir, ai_material)).first).second;
		}

		void updateUBO(int curentFrame, glm::mat4 modelMat, const glm::mat4& view, const glm::mat4& proj)
		{
			UniformBuffers::UniformBufferObject ubo{};
			ubo.model = modelMat;
			ubo.view = view;
			ubo.proj = proj;

			ubo.proj[1][1] *= -1;

			void* data = uniform_buffers_.getUBMemory(curentFrame).mapMemory(0, sizeof(ubo));
			memcpy(data, &ubo, sizeof(ubo));
			uniform_buffers_.getUBMemory(curentFrame).unmapMemory();
		}

		static void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			un_lit_descriptors_statics_.setRenderPass(render_pass);
		}

		Texture diffuse;
		UniformBuffers uniform_buffers_;
		UnLitTexturedDescriptorSets descriptor_sets_;
		static inline UnLitTexturedDescriptorsStatics un_lit_descriptors_statics_{};

	private:
		// checks all material textures of a given type and loads the textures if they're not loaded yet.
		// the required info is returned as a Texture struct.
		std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string directory)
		{
			std::vector<Texture> textures;
			for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
			{
				aiString str;
				mat->GetTexture(type, i, &str);
				std::string filename = directory + '\\' + std::string(str.C_Str());

				textures.emplace_back(Texture{filename});
			}
			return textures;
		}

		std::string getDeffuseTexturePath(const aiMaterial* mat, aiTextureType type, const std::string directory)
		{
			aiString str;
			mat->GetTexture(type, 0, &str);
			std::string filename = directory + '\\' + std::string(str.C_Str());
			return filename;
		}
	};
}
