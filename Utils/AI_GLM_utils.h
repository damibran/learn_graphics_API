#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <assimp/scene.h>
#include <assimp/matrix4x4.h>
#include <assimp/vector3.h>

namespace dmbrn
{
	glm::vec3 toGlm(const aiVector3D& vec);

	glm::quat toGlm(const aiQuaternion& vec);

	glm::mat4 toGlm(const aiMatrix4x4& mat);

	glm::vec3 getScale(const glm::mat4 mat);

	glm::vec3 operator/(const glm::vec3& vec, const float s);

	void printAiScene(const aiScene* scene);

	void printAnimations(const aiScene* scene);

}
