#include "AI_GLM_utils.h"

#include <iostream>
#include <stack>

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
