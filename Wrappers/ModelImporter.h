#pragma once

#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>

#include "Mesh.h"
#include "Utils/Transform.h"

namespace dmbrn
{
	glm::vec3 toGlm(const aiVector3D& vec)
	{
		return {vec.x, vec.y, vec.z};
	}

	glm::mat4 toGlm(const aiMatrix4x4& mat)
	{
		return glm::mat4
		{
			mat.a1, mat.b1, mat.c1, mat.d1,
			mat.a2, mat.b2, mat.c2, mat.d2,
			mat.a3, mat.b3, mat.c3, mat.d3,
			mat.a4, mat.b4, mat.c4, mat.d4,
		};
	}

	glm::vec3 operator/(const glm::vec3& vec, const float s)
	{
		return {vec.x / s, vec.y / s, vec.z / s};
	}

	std::string toString(const aiMatrix4x4& mat)
	{
		std::string res;
		for (int i = 0; i < 4; ++i)
		{
			auto vec = mat[i];
			for (int j = 0; j < 4; ++j)
			{
				res += std::to_string(vec[j]) + " ";
			}
			res += "\n";
		}
		return res;
	}


}
