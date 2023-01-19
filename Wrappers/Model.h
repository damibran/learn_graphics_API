#pragma once

#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>

#include "Mesh.h"
#include "Texture.h"
#include "MaterialSystem/ShaderEffects/ShaderEffect.h"

namespace dmbrn
{
	class Model
	{
	public:
		inline static std::unordered_map<std::string, Model> model_instances;
		// model data 
		std::vector<Mesh> meshes;
		std::string directory;
		std::string name;
		std::string extension;

		~Model()=default;
		Model(const Model&) = delete;
		Model(Model&&) = delete;

		// constructor, expects a filepath to a 3D model.
		Model(const std::string& path)
		{
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
			directory = path.substr(0, path.find_last_of('\\'));
			name = path.substr(path.find_last_of('\\') + 1, path.find_last_of('.') - path.find_last_of('\\') - 1);
			extension = path.substr(path.find_last_of('.'));

			// process ASSIMP's root node recursively
			processNode(scene->mRootNode, scene, aiMatrix4x4{});
		}

		std::string getPath()const
		{
			return directory + "\\" + name + extension;
		}

	private:
		// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
		void processNode(aiNode* node, const aiScene* scene, const aiMatrix4x4& parentTransform)
		{
			aiMatrix4x4 trans_this = parentTransform * node->mTransformation;
			// process each mesh located at the current node
			for (unsigned int i = 0; i < node->mNumMeshes; i++)
			{
				// the node object only contains indices to index the actual objects in the scene. 
				// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
				meshes.emplace_back(directory, name, material, mesh, trans_this);
			}
			// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
			for (unsigned int i = 0; i < node->mNumChildren; i++)
			{
				processNode(node->mChildren[i], scene, trans_this);
			}
		}
	};
}
