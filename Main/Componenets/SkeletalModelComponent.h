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

		SkeletalModelComponent(Enttity skel_ent, SkeletalMesh&& mesh,
		                       ShaderEffect* shader = nullptr) :
			skeleton_ent(skel_ent),
			mesh(std::move(mesh)),
			shader_(shader)
		{
		}

		ShaderEffect* getShader()
		{
			return shader_;
		}

		Enttity skeleton_ent;
		SkeletalMesh mesh;
		ShaderEffect* shader_ = nullptr;
	};
}
