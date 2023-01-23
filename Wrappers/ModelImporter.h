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

	class ModelImporter
	{
	public:
		// constructor, expects a filepath to a 3D model.
		static SceneNode Import(const std::string& path)
		{
			SceneNode res;
			// read file via ASSIMP
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);
			//| aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
			// check for errors
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
			{
				throw std::runtime_error(std::string("ERROR::ASSIMP:: ") + importer.GetErrorString());
			}
			// retrieve the directory path of the filepath
			std::string directory = path.substr(0, path.find_last_of('\\'));
			std::string model_name = path.substr(path.find_last_of('\\') + 1,
			                                     path.find_last_of('.') - path.find_last_of('\\') - 1);

			// process ASSIMP's root node recursively
			processNode(res, scene->mRootNode, scene, directory, model_name);

			res.name = model_name;

			return res;
		}

	private:
		// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
		static void processNode(SceneNode& node, aiNode* ai_node, const aiScene* scene,
		                        const std::string& directory,
		                        const std::string& parentName)
		{
			node.name = ai_node->mName.C_Str();
			std::string name_this = parentName + "." + ai_node->mName.C_Str();
			// process each mesh located at the current node
			for (unsigned int i = 0; i < ai_node->mNumMeshes; i++)
			{
				// the node object only contains indices to index the actual objects in the scene. 
				// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
				
				aiMesh* mesh = scene->mMeshes[ai_node->mMeshes[i]];
				std::string mesh_name = name_this + + "." + std::string(mesh->mName.C_Str());
				aiMaterial* ai_material = scene->mMaterials[mesh->mMaterialIndex];
				Material* material = DiffusionMaterial::GetMaterialPtr(directory, mesh_name, ai_material);

				aiVector3D translation;
				aiVector3D orientation;
				aiVector3D scale;

				ai_node->mTransformation.Decompose(scale, orientation, translation);

				node.children.push_back(SceneNode{
					mesh->mName.C_Str(),
					Transform{toGlm(translation) / 100, toGlm(orientation), toGlm(scale) / 100},
					Mesh::GetMeshPtr(material, mesh_name, mesh)
				});
			}
			// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
			for (unsigned int i = 0; i < ai_node->mNumChildren; i++)
			{
				node.children.push_back(SceneNode{});
				processNode(node.children.back(), ai_node->mChildren[i], scene, directory, name_this);
			}
		}
	};
}
