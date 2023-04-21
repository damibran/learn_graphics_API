#pragma once
#include <iomanip>
#include <assimp/scene.h>
#include <assimp/texture.h>

#include "Utils/StdUtils.h"

#include "DiffusionUniformBuffer.h"
#include"MaterialSystem/Materials/Material.h"
#include"MaterialSystem/Materials/Diffusion/DiffusionDescriptorSets.h"
#include"Wrappers/Texture.h"

namespace dmbrn
{
	class DiffusionMaterial : public Material
	{
		struct MaterialRegistryHandle
		{
			image_data diffuse_texture;
			glm::vec4 base_color;

			struct hash
			{
				size_t operator()(const MaterialRegistryHandle& handle) const noexcept
				{
					std::hash<image_data> img_hasher;
					std::hash<glm::vec4> vec_hasher;
					size_t res = img_hasher(handle.diffuse_texture) ^ vec_hasher(handle.base_color);
					std::cout << "Hash: " << res << std::endl;
					return res;
				}
			};

			bool operator==(const MaterialRegistryHandle& other) const
			{
				return base_color == other.base_color && diffuse_texture == other.diffuse_texture;
			}
		};

	public:
		DiffusionMaterial() = delete;

		void bindMaterialData(int frame, const vk::raii::CommandBuffer& command_buffer,
		                      vk::PipelineLayout layout) const override
		{
			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			                                  layout, 2,
			                                  *descriptor_sets_[frame], nullptr);
		}

		static Material* GetMaterialPtr(const std::string& directory, const aiScene* scene,
		                                const aiMaterial* ai_material)
		{
			std::cout << "\nNew mat\n";
			MaterialRegistryHandle handle = getHandle(directory, scene, ai_material);

			auto it = material_registry.find(handle);
			if (it == material_registry.end())
			{
				std::cout << "didn't found\n";
				it = material_registry.emplace(std::move(handle), DiffusionMaterial{directory, scene, ai_material}).
				                       first;
			}
			else
			{
				std::cout << "found\n";
			}

			return &it->second;
		}

		static size_t getRegistrySize()
		{
			return material_registry.size();
		}

	private:
		DiffusionMaterial(const std::string& directory, const aiScene* scene,
		                  const aiMaterial* ai_material) :
			diffuse(getTexture(aiTextureType_DIFFUSE, 0, directory, scene, ai_material),true),
			base_color(Singletons::physical_device, Singletons::device, getBaseColor(ai_material)),
			descriptor_sets_(Singletons::device, diffuse, base_color)
		{
		}

		static MaterialRegistryHandle getHandle(const std::string& directory, const aiScene* scene,
		                                        const aiMaterial* ai_material)
		{
			MaterialRegistryHandle h;

			h.diffuse_texture = getTexture(aiTextureType_DIFFUSE, 0, directory, scene, ai_material);
			h.base_color = getBaseColor(ai_material);

			return h;
		}

		static glm::vec4 getBaseColor(const aiMaterial* ai_material)
		{
			aiColor4D c;
			ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, c); // or AI_MATKEY_BASE_COLOR on first glance are same
			return {c.r, c.g, c.b, c.a};
		}

		[[nodiscard]] static image_data getTexture(aiTextureType type, int index, const std::string directory,
		                                           const aiScene* scene,
		                                           const aiMaterial* ai_material)
		{
			image_data res;
			aiString s;
			ai_material->Get(AI_MATKEY_TEXTURE(type, index), s);

			if (s.length == 0)
			{
				std::array<unsigned char, 4> white = {255, 255, 255, 255};

				res.width = 1;
				res.height = 1;
				res.comp_per_pix = 4;

				res.copyData(white.data(), res.getLength());

				stbi_info_from_memory(white.data(), 3, &res.width, &res.height,
				                      &res.comp_per_pix);
			}
			else if (auto dif_texture = scene->GetEmbeddedTexture(s.C_Str()))
			{
				if (dif_texture->mHeight == 0)
				{
					stbi_uc* temp = stbi_load_from_memory(reinterpret_cast<unsigned char*>(dif_texture->pcData),
					                                      dif_texture->mWidth, &res.width, &res.height,
					                                      &res.comp_per_pix,
					                                      STBI_rgb_alpha);
					res.comp_per_pix = 4; // because req_comp = STBI_rgb_alpha

					if (!temp)
						//throw std::runtime_error("failed to load texture image! " + std::string(s.C_Str()));
						abort();

					res.copyData(temp, res.getLength());

					stbi_image_free(temp);
				}
				else
				{
					stbi_uc* temp = stbi_load_from_memory(reinterpret_cast<unsigned char*>(dif_texture->pcData),
					                                      dif_texture->mWidth * dif_texture->mHeight, &res.width,
					                                      &res.height,
					                                      &res.comp_per_pix, STBI_rgb_alpha);

					res.comp_per_pix = 4; // because req_comp = STBI_rgb_alpha

					if (!temp)
						//throw std::runtime_error("failed to load texture image! " + std::string(s.C_Str()));
						abort();

					res.copyData(temp, res.getLength());

					stbi_image_free(temp);
				}
			}
			else
			{
				std::string path = directory + "\\" + std::string(s.C_Str());

				stbi_uc* temp = stbi_load(path.c_str(), &res.width, &res.height, &res.comp_per_pix, STBI_rgb_alpha);
				res.comp_per_pix = 4; // because req_comp = STBI_rgb_alpha

				if (!temp)
					//throw std::runtime_error("failed to load texture image! " + std::string(s.C_Str()));
					abort();

				res.copyData(temp, res.getLength());

				stbi_image_free(temp);
			}


			return res;
		}

		Texture diffuse;
		DiffusionUniformBuffer base_color;
		DiffusionDescriptorSets descriptor_sets_;
		static inline std::unordered_map<MaterialRegistryHandle, DiffusionMaterial, MaterialRegistryHandle::hash>
		material_registry;
	};
}
