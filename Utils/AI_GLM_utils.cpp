#include "AI_GLM_utils.h"

#include <iostream>
#include <stack>
#include <glm/gtx/matrix_decompose.hpp>

namespace dmbrn
{
	glm::vec3 toGlm(const aiVector3D& vec)
	{
		return glm::vec3{vec.x, vec.y, vec.z};
	}

	glm::quat toGlm(const aiQuaternion& quat)
	{
		return glm::quat{quat.w,quat.x,quat.y,quat.z};
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

	glm::vec3 getScale(const glm::mat4 mat)
	{
		glm::vec3 Row[3];
		glm::vec3 Scale,Skew;

		for(glm::length_t i = 0; i < 3; ++i)
		for(glm::length_t j = 0; j < 3; ++j)
			Row[i][j] = mat[i][j];

		// Compute X scale factor and normalize first row.
		Scale.x = length(Row[0]);// v3Length(Row[0]);

		Row[0] = glm::detail::scale(Row[0], static_cast<float>(1));

		// Compute XY shear factor and make 2nd row orthogonal to 1st.
		Skew.z = dot(Row[0], Row[1]);
		Row[1] = glm::detail::combine(Row[1], Row[0], static_cast<float>(1), -Skew.z);

		// Now, compute Y scale and normalize 2nd row.
		Scale.y = length(Row[1]);
		Row[1] = glm::detail::scale(Row[1], static_cast<float>(1));

		// Compute XZ and YZ shears, orthogonalize 3rd row.
		Skew.y = glm::dot(Row[0], Row[2]);
		Row[2] =glm::detail::combine(Row[2], Row[0], static_cast<float>(1), -Skew.y);
		Skew.x = glm::dot(Row[1], Row[2]);
		Row[2] = glm::detail::combine(Row[2], Row[1], static_cast<float>(1), -Skew.x);

		// Next, get Z scale and normalize 3rd row.
		Scale.z = length(Row[2]);
		Row[2] = glm::detail::scale(Row[2], static_cast<float>(1));

		glm::vec3 Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
		if(dot(Row[0], Pdum3) < 0)
		{
			for(glm::length_t i = 0; i < 3; i++)
			{
				Scale[i] *= static_cast<float>(-1);
				Row[i] *= static_cast<float>(-1);
			}
		}

		return Scale;
	}

	glm::vec3 operator/(const glm::vec3& vec, const float s)
	{
		return {vec.x / s, vec.y / s, vec.z / s};
	}



	void printAiScene(const aiScene* scene)
	{
		std::stack<std::pair<std::string, aiNode*>> stack;

		stack.push({"", scene->mRootNode});

		std::cout << "Scene hierarchy\n";
		while (!stack.empty())
		{
			auto [intend, node] = stack.top();
			stack.pop();

			std::cout << intend + "Node name:" << node->mName.C_Str() << std::endl;

			for (unsigned i = 0; i < node->mNumChildren; ++i)
			{
				stack.push({intend + "|  ", node->mChildren[i]});
			}
		}
	}

	void printAnimations(const aiScene* ai_scene)
	{
		for (int i = 0; i < ai_scene->mNumAnimations; ++i)
		{
			aiAnimation* ai_animation = ai_scene->mAnimations[i];

			std::cout << "Animation name:" << ai_animation->mName.C_Str() << std::endl;

			for (int j = 0; j < ai_animation->mNumChannels; ++j)
			{
				aiNodeAnim* ai_node_anim = ai_animation->mChannels[j];
				std::cout << "  AnimNode name: " << ai_node_anim->mNodeName.C_Str() << std::endl;
			}
		}
	}
}
