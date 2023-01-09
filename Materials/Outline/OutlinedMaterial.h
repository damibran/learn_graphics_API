#pragma once
#include <assimp/material.h>
#include "Materials/Material.h"
#include "Materials/UnLitTextured/UnLitTexturedDescriptorsStatics.h"
#include "Wrappers/UniformBuffers.h"

namespace dmbrn
{
	template<class M, typename ... Args>
	struct OutlinedMaterial : Material
	{
		~OutlinedMaterial() override = default;

		OutlinedMaterial(OutlinedMaterial&& other) = default;
		OutlinedMaterial& operator=(OutlinedMaterial&& other) = default;

		OutlinedMaterial(const OutlinedMaterial& other) = delete;
		OutlinedMaterial& operator=(const OutlinedMaterial& other) = delete;

		OutlinedMaterial(float scale_factor, Args&& ...args):
			scale_factor_(scale_factor),
		stencil_mat(M<true>(std::forward<Args>(args))) // should support stencil buff
		{
		}
	private:
		float scale_factor_=1.f;
		M<true> stencil_mat;
	};
}
