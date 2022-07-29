#pragma once

#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>

#include "Mesh.h"
#include "Texture.h"


//unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

namespace dmbrn
{
	class Model
	{
	public:
		// model data 
		std::vector<Mesh> meshes;
		std::string directory;

		// constructor, expects a filepath to a 3D model.
		Model(const std::string& path, const PhysicalDevice& physical_device, const LogicalDevice& device,
		      const CommandPool& command_pool, vk::raii::Queue gragraphics_queue)
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

			// process ASSIMP's root node recursively
			processNode(scene->mRootNode, scene, physical_device, device, command_pool, gragraphics_queue);
		}

		// draws the model, and thus all its meshes
		void Draw(int frame, const LogicalDevice& device, const GraphicsPipeline& graphics_pipeline,
		          const vk::raii::CommandBuffer& command_buffers, const DescriptorSets& descriptor_sets) const
		{
			for (unsigned int i = 0; i < meshes.size(); i++)
				meshes[i].Draw(frame, device, graphics_pipeline, command_buffers, descriptor_sets);
		}

	private:
		// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
		void processNode(aiNode* node, const aiScene* scene, const PhysicalDevice& physical_device,
		                 const LogicalDevice& device,
		                 const CommandPool& command_pool, vk::raii::Queue gragraphics_queue)
		{
			// process each mesh located at the current node
			for (unsigned int i = 0; i < node->mNumMeshes; i++)
			{
				// the node object only contains indices to index the actual objects in the scene. 
				// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				meshes.push_back(processMesh(mesh, scene, physical_device, device, command_pool, gragraphics_queue));
			}
			// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
			for (unsigned int i = 0; i < node->mNumChildren; i++)
			{
				processNode(node->mChildren[i], scene, physical_device, device, command_pool, gragraphics_queue);
			}
		}

		Mesh processMesh(aiMesh* mesh, const aiScene* scene, const PhysicalDevice& physical_device,
		                 const LogicalDevice& device,
		                 const CommandPool& command_pool, vk::raii::Queue gragraphics_queue)
		{
			// data to fill
			std::vector<Vertex> vertices;
			std::vector<uint16_t> indices;
			std::vector<Texture> textures;

			// walk through each of the mesh's vertices
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				Vertex vertex;
				glm::vec3 vector;
				// we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
				// positions
				vector.x = mesh->mVertices[i].x;
				vector.y = mesh->mVertices[i].y;
				vector.z = mesh->mVertices[i].z;
				vertex.pos = vector;
				// normals
				//if (mesh->HasNormals())
				//{
				//	vector.x = mesh->mNormals[i].x;
				//	vector.y = mesh->mNormals[i].y;
				//	vector.z = mesh->mNormals[i].z;
				//	vertex. = vector;
				//}
				// texture coordinates
				if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
				{
					glm::vec2 vec;
					// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
					// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
					vec.x = mesh->mTextureCoords[0][i].x;
					vec.y = mesh->mTextureCoords[0][i].y;
					vertex.texCoord = vec;
					// tangent
					//vector.x = mesh->mTangents[i].x;
					//vector.y = mesh->mTangents[i].y;
					//vector.z = mesh->mTangents[i].z;
					//vertex.Tangent = vector;
					//// bitangent
					//vector.x = mesh->mBitangents[i].x;
					//vector.y = mesh->mBitangents[i].y;
					//vector.z = mesh->mBitangents[i].z;
					//vertex.Bitangent = vector;
				}
				else
					vertex.texCoord = glm::vec2(0.0f, 0.0f);

				vertices.push_back(vertex);
			}
			// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
			for (unsigned int i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				// retrieve all indices of the face and store them in the indices vector
				for (unsigned int j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j]);
			}
			// process materials
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
			// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
			// Same applies to other texture as the following list summarizes:
			// diffuse: texture_diffuseN
			// specular: texture_specularN
			// normal: texture_normalN

			// 1. diffuse maps
			std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse",
			                                                        physical_device, device, command_pool,
			                                                        gragraphics_queue);
			textures.insert(textures.end(), std::make_move_iterator(diffuseMaps.begin()),
			                std::make_move_iterator(diffuseMaps.end()));
			// 2. specular maps
			//std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			//textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			//// 3. normal maps
			//std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
			//textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
			//// 4. height maps
			//std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
			//textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

			// return a mesh object created from the extracted mesh data
			return Mesh(std::move(vertices), std::move(indices), std::move(textures), physical_device, device,
			            command_pool, gragraphics_queue);
		}

		// checks all material textures of a given type and loads the textures if they're not loaded yet.
		// the required info is returned as a Texture struct.
		std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName,
		                                          const PhysicalDevice& physical_device, const LogicalDevice& device,
		                                          const CommandPool& command_pool, vk::raii::Queue gragraphics_queue)
		{
			std::vector<Texture> textures;
			for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
			{
				aiString str;
				mat->GetTexture(type, i, &str);
				std::string filename = directory + '\\' + std::string(str.C_Str());

				textures.emplace_back(Texture{filename, physical_device, device, command_pool, gragraphics_queue});
			}
			return textures;
		}
	};


	/*unsigned int TextureFromFile(const char *path, const string &directory, bool gamma)
	{
		string filename = string(path);
		filename = directory + '/' + filename;

		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}

		return textureID;
	}*/
}
