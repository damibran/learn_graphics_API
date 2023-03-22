#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "Utils/StdUtils.h"

#include <assimp/material.h>
#include <assimp/mesh.h>
#include<glm/glm.hpp>
#include"MaterialSystem/Materials/Diffusion/DiffusionMaterial.h"
#include "Texture.h"
#include "Vertex.h"
#include "Wrappers/HostLocalBuffer.h"

namespace dmbrn
{
	class Mesh
	{
	public:
		Mesh() = default;

		Mesh(const Mesh& other)=delete;
		const Mesh& operator=(const Mesh& other)=delete;

		Mesh(Mesh&& other) = default;
		Mesh& operator=(Mesh&& other) = default;

		const Material* material_=nullptr;

		Mesh(const Material* material, const std::string& full_mesh_name, const aiMesh* mesh)
		{
			material_ = material;
			render_data_ = MeshRenderData::GetRenderDataPtr(full_mesh_name, mesh);
		}

		void bind(const vk::raii::CommandBuffer& command_buffer) const
		{
			render_data_->bind(command_buffer);
		}

		void drawIndexed(const vk::raii::CommandBuffer& command_buffer)const
		{
			command_buffer.drawIndexed(render_data_->indices_count, 1, 0, 0, 0);
		}

		const class MeshRenderData
		{
			friend class Mesh;
		public:
			static MeshRenderData* GetRenderDataPtr(const std::string& full_mesh_name, const aiMesh* mesh)
			{
				std::vector<aiVector3D> temp;
				std::insert_iterator insrt_it{temp, temp.begin()};
				std::copy_n(mesh->mVertices, mesh->mNumVertices, insrt_it);

				auto it = registry_.find(temp);
				if (it == registry_.end())
					it = registry_.emplace(temp, MeshRenderData{full_mesh_name, mesh}).first;
				else
					it->second.use_this_mesh_.push_back(full_mesh_name);

				return &it->second;
			}

			static size_t getRegistrySize()
			{
				return registry_.size();
			}

		private:
			static inline std::unordered_map<std::vector<aiVector3D>, MeshRenderData> registry_;
			uint32_t indices_count;
			std::vector<std::string> use_this_mesh_;
			HostLocalBuffer<Vertex> vertex_buffer_;
			HostLocalBuffer<uint16_t> index_buffer_;

			MeshRenderData(const std::string& mesh_name,const aiMesh* mesh):
				use_this_mesh_({mesh_name})
			{
				auto [vertices,indices] = getDataFromMesh(mesh);
				indices_count = indices.size();

				vertex_buffer_ = HostLocalBuffer(vertices, vk::BufferUsageFlagBits::eVertexBuffer);

				index_buffer_ = HostLocalBuffer(indices,vk::BufferUsageFlagBits::eIndexBuffer);
			}

			std::pair<std::vector<Vertex>, std::vector<uint16_t>> getDataFromMesh(const aiMesh* mesh)
			{
				std::vector<Vertex> vertices;
				std::vector<uint16_t> indices;
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
					if (mesh->HasNormals())
					{
						vector.x = mesh->mNormals[i].x;
						vector.y = mesh->mNormals[i].y;
						vector.z = mesh->mNormals[i].z;
						vertex.normal = vector;
					}
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

				return {vertices, indices};
			}

			void bind(const vk::raii::CommandBuffer& command_buffer) const
			{
				command_buffer.bindVertexBuffers(0, vertex_buffer_.getBuffer(), {0});

				command_buffer.bindIndexBuffer(index_buffer_.getBuffer(), 0, vk::IndexType::eUint16);
			}
		}* render_data_=nullptr;
	};
}
