#pragma once
#include "MaterialSystem/ShaderEffects/ShaderEffect.h"
#include "Wrappers/SkeletalMesh.h"
#include "Wrappers/Singletons/Renderer.h"

namespace dmbrn
{
	// TODO proper unregister
	struct SkeletalModelComponent
	{
		SkeletalModelComponent() = default;

		SkeletalModelComponent(SkeletalMesh&& mesh, std::vector<Enttity> bone_entts,
		                       ShaderEffect* shader = nullptr) :
			in_GPU_mtxs_offset(Renderer::per_skeleton_data_.registerObject()),
			mesh(std::move(mesh)),
			bone_enttities(bone_entts),
			shader_(shader)
		{
		}

		ShaderEffect* getShader()
		{
			return shader_;
		}

		uint32_t in_GPU_mtxs_offset;
		SkeletalMesh mesh;
		std::vector<Enttity> bone_enttities;
		ShaderEffect* shader_ = nullptr;
	};
}
