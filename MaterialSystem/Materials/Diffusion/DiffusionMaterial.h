#pragma once
#include <assimp/scene.h>
#include <assimp/texture.h>

#include"MaterialSystem/Materials/Material.h"
#include"MaterialSystem/Materials/Diffusion/DiffusionDescriptorSets.h"
#include"Wrappers/Texture.h"

namespace std
{
	template <>
	struct hash<std::vector<stbi_uc>>
	{
		size_t operator()(const std::vector<stbi_uc>& vertices) const noexcept
		{
			std::hash<stbi_uc> hasher;
			size_t res = 0;
			for (const auto& vec : vertices)
			{
				res ^= hasher(vec);
			}
			return res;
		}
	};
}

namespace dmbrn
{
	class DiffusionMaterial : public Material
	{
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
			image_data image_data = getTexture(aiTextureType_DIFFUSE, 0, directory, scene, ai_material);

			std::vector<unsigned char> temp_vec;
			std::insert_iterator insrt_it{temp_vec, temp_vec.begin()};

			std::copy_n(image_data.data.get(), image_data.height * image_data.width * image_data.comp_per_pix,
			            insrt_it);

			auto it = material_registry.find(temp_vec);
			if (it == material_registry.end())
				it = material_registry.emplace(temp_vec, DiffusionMaterial{directory, scene, ai_material}).first;

			return &it->second;
		}

		static size_t getRegistrySize()
		{
			return material_registry.size();
		}

	private:
		DiffusionMaterial(const std::string& directory, const aiScene* scene,
		                  const aiMaterial* ai_material) :
			diffuse(getTexture(aiTextureType_DIFFUSE, 0, directory, scene, ai_material)),
			descriptor_sets_(Singletons::device, diffuse)
		{
		}

		[[nodiscard]] static image_data getTexture(aiTextureType type, int index, const std::string directory,
		                                           const aiScene* scene,
		                                           const aiMaterial* ai_material)
		{
			image_data res;
			aiString s;
			ai_material->Get(AI_MATKEY_TEXTURE(type, index), s);
			if (auto dif_texture = scene->GetEmbeddedTexture(s.C_Str()))
			{
				if (dif_texture->mHeight == 0)
				{
					res.data.reset(stbi_load_from_memory(reinterpret_cast<unsigned char*>(dif_texture->pcData),
					                                     dif_texture->mWidth, &res.width, &res.height,
					                                     &res.comp_per_pix,
					                                     STBI_rgb_alpha));
				}
				else
				{
					res.data.reset(stbi_load_from_memory(reinterpret_cast<unsigned char*>(dif_texture->pcData),
					                                     dif_texture->mWidth * dif_texture->mHeight, &res.width,
					                                     &res.height,
					                                     &res.comp_per_pix, STBI_rgb_alpha));
				}
			}
			else
			{
				std::string path = directory + "\\" + std::string(s.C_Str());
				res.data.reset(stbi_load(path.c_str(), &res.width, &res.height, &res.comp_per_pix, STBI_rgb_alpha));
			}

			if (!res.data.get())
				//throw std::runtime_error("failed to load texture image! " + std::string(s.C_Str()));
				abort();

			res.comp_per_pix = 4; // because req_comp = STBI_rgb_alpha

			return res;
		}

		Texture diffuse;
		DiffusionDescriptorSets descriptor_sets_;
		static inline std::unordered_map<std::vector<stbi_uc>, DiffusionMaterial> material_registry;
	};
}
