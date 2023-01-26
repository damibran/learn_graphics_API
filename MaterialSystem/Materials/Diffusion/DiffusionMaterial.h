#pragma once
#include"MaterialSystem/Materials/Material.h"
#include"MaterialSystem/Materials/Diffusion/DiffusionDescriptorSets.h"
#include"Wrappers/Texture.h"

namespace dmbrn
{
	class DiffusionMaterial :public Material
	{
	public:
		DiffusionMaterial() = delete;

		void bindMaterialData(int frame, const vk::raii::CommandBuffer& command_buffer, vk::PipelineLayout layout)const
		{
			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
				layout, 2,
				*descriptor_sets_[frame], nullptr);

		}
		static Material* GetMaterialPtr(const std::string& dir, const std::string& model_name, const aiMaterial* ai_material)
		{
			std::string material_name = model_name + "." + std::string{ ai_material->GetName().data };

			auto it = material_registry.find(material_name);
			if (it == material_registry.end())
				it = material_registry.emplace(material_name, DiffusionMaterial{ dir, ai_material }).first;

			return &it->second;
		}

	private:

		DiffusionMaterial(const std::string& dir,
			const aiMaterial* ai_material) :
			diffuse(getDiffuseTexturePath(ai_material, aiTextureType_DIFFUSE, dir)),
			descriptor_sets_(Singletons::device, diffuse)
		{
		}

		// TODO: move this into Texture class, enum texture types
		std::string getDiffuseTexturePath(const aiMaterial* mat, aiTextureType type, const std::string& directory)
		{
			aiString str;
			mat->GetTexture(type, 0, &str);
			std::string filename = directory + '\\' + std::string(str.C_Str());
			return filename;
		}

		Texture diffuse;
		DiffusionDescriptorSets descriptor_sets_;
		static inline std::unordered_map<std::string, DiffusionMaterial> material_registry;
	};
}
