#pragma once

#include <glm/glm.hpp>

#include <string>
#include <assimp/scene.h>
#include <assimp/matrix4x4.h>
#include <assimp/vector3.h>

namespace dmbrn
{
	// TODO merge toGLM
	glm::vec3 toGlm(const aiVector3D& vec);

	glm::mat4 toGlm(const aiMatrix4x4& mat);

	glm::vec3 operator/(const glm::vec3& vec, const float s);

	std::string toString(const aiMatrix4x4& mat);

	void printAiScene(const aiScene* scene);

	void printAnimations(const aiScene* scene);

}
