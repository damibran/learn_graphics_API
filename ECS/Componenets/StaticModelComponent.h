#pragma once
#include "MaterialSystem/ShaderEffects/ShaderEffect.h"
#include "Wrappers/Mesh.h"
#include "Wrappers/Singletons/Renderer.h"

namespace dmbrn
{
	/**
	 * \brief describes an entity with static model visual representation
	 */
	struct StaticModelComponent
	{
		StaticModelComponent() = default;

		StaticModelComponent(Mesh&& mesh, ShaderEffect* shader = nullptr) :
			mesh(std::move(mesh)),
			shader_(shader),
			inGPU_transform_offset(Renderer::per_static_data_buffer_.registerObject())
		{
		}

		ShaderEffect* getShader()
		{
			return shader_;
		}

		Mesh mesh;
		ShaderEffect* shader_ = nullptr;
		uint32_t inGPU_transform_offset=0;
		bool need_GPU_state_update = true;
	};
}
