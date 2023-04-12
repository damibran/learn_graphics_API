#pragma once
#include "MaterialSystem/ShaderEffects/ShaderEffect.h"
#include "Wrappers/SkeletalMesh.h"
#include "Wrappers/Singletons/Renderer.h"

namespace dmbrn
{
	// TODO proper unregister
	struct SkeletalModelComponent
	{
		SkeletalModelComponent()=default;

		SkeletalModelComponent(SkeletalMesh&& mesh, ShaderEffect* shader = nullptr) :
			in_GPU_mtxs_offset(Renderer::per_skeleton_data_.registerObject()),
			mesh(std::move(mesh)),
			shader_(shader)
		{
		}

		ShaderEffect* getShader()
		{
			return shader_;
		}

		size_t in_GPU_mtxs_offset;
		SkeletalMesh mesh;
		ShaderEffect* shader_ = nullptr;
	};
}
