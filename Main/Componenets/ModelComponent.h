#pragma once
#include "MaterialSystem/ShaderEffects/ShaderEffect.h"
#include "Wrappers/Mesh.h"

namespace dmbrn
{
	struct ModelComponent
	{
		ModelComponent() = default;

		ModelComponent(Mesh&& mesh, ShaderEffect* shader = nullptr) :
			mesh(std::move(mesh)),
			shader_(shader)
		{
		}

		ShaderEffect* getShader()
		{
			return shader_;
		}

		Mesh mesh;
		ShaderEffect* shader_ = nullptr;
	};
}
