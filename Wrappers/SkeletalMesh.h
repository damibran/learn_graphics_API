#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <assimp/mesh.h>
#include<glm/glm.hpp>
#include "Utils/AI_GLM_utils.h"
#include"MaterialSystem/Materials/Diffusion/DiffusionMaterial.h"
#include "BonedVertex.h"
#include "Wrappers/HostLocalBuffer.h"

namespace dmbrn
{
	class SkeletalMesh
	{
	public:
		SkeletalMesh() = default;

		SkeletalMesh(const SkeletalMesh& other) = delete;
		const SkeletalMesh& operator=(const SkeletalMesh& other) = delete;

		SkeletalMesh(SkeletalMesh&& other) = default;
		SkeletalMesh& operator=(SkeletalMesh&& other) = default;

		const Material* material_ = nullptr;

		SkeletalMesh(const Material* material, const std::string& full_mesh_name, const aiMesh* mesh,
		             const std::unordered_map<uint32_t, uint32_t>& local_to_global_bone_ind)
		{
			material_ = material;
			render_data_ = SkeletalMeshRenderData::GetRenderDataPtr(full_mesh_name, mesh, local_to_global_bone_ind);
		}

		void bind(const vk::raii::CommandBuffer& command_buffer) const
		{
			render_data_->bind(command_buffer);
		}

		void drawIndexed(const vk::raii::CommandBuffer& command_buffer) const
		{
			command_buffer.drawIndexed(render_data_->indices_count, 1, 0, 0, 0);
		}

		const class SkeletalMeshRenderData
		{
			friend class SkeletalMesh;
		public:
			static SkeletalMeshRenderData* GetRenderDataPtr(const std::string& full_mesh_name, const aiMesh* mesh,
			                                                const std::unordered_map<uint32_t, uint32_t>&
			                                                local_to_global_bone_ind)
			{
				std::vector<aiVector3D> temp;
				std::insert_iterator insrt_it{temp, temp.begin()};
				std::copy_n(mesh->mVertices, mesh->mNumVertices, insrt_it);

				auto it = registry_.find(temp);
				if (it == registry_.end())
					it = registry_.emplace(temp, SkeletalMeshRenderData{
						                       full_mesh_name, mesh, local_to_global_bone_ind
					                       }).first;
				else
					it->second.use_this_mesh_.push_back(full_mesh_name);

				return &it->second;
			}

			static size_t getRegistrySize()
			{
				return registry_.size();
			}

		private:
			static inline std::unordered_map<std::vector<aiVector3D>, SkeletalMeshRenderData> registry_;
			uint32_t indices_count;
			std::vector<std::string> use_this_mesh_;
			HostLocalBuffer<BonedVertex> vertex_buffer_;
			HostLocalBuffer<uint16_t> index_buffer_;

			SkeletalMeshRenderData(const std::string& mesh_name, const aiMesh* mesh,
			                       const std::unordered_map<uint32_t, uint32_t>& local_to_global_bone_ind):
				use_this_mesh_({mesh_name})
			{
				const auto&& [vertices,indices] = getDataFromMesh(mesh, local_to_global_bone_ind);
				indices_count = static_cast<uint32_t>(indices.size());

				vertex_buffer_ = HostLocalBuffer(vertices, vk::BufferUsageFlagBits::eVertexBuffer);

				index_buffer_ = HostLocalBuffer(indices, vk::BufferUsageFlagBits::eIndexBuffer);
			}

			std::pair<std::vector<BonedVertex>, std::vector<uint16_t>> getDataFromMesh(
				const aiMesh* mesh, const std::unordered_map<uint32_t, uint32_t>& local_to_global_bone_ind)
			{
				std::vector<BonedVertex> vertices;
				std::vector<unsigned int> vertex_bone_count(mesh->mNumVertices, 0);
				std::vector<uint16_t> indices;

				// walk through each of the mesh's vertices
				for (unsigned int i = 0; i < mesh->mNumVertices; i++)
				{
					BonedVertex vertex;
					glm::vec3 vector;

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
					if (mesh->mTextureCoords[0])
					{
						glm::vec2 vec;
						vec.x = mesh->mTextureCoords[0][i].x;
						vec.y = mesh->mTextureCoords[0][i].y;
						vertex.texCoord = vec;
					}

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

				// iterate all bones
				for (unsigned int i = 0; i < mesh->mNumBones; ++i)
				{
					const aiBone* bone = mesh->mBones[i];

					// iterate all weights, add bone id (i) and weight to corresponding vertex
					for (unsigned int j = 0; j < bone->mNumWeights; ++j)
					{
						const aiVertexWeight vw = bone->mWeights[j];

						unsigned int cur_vrtx_bone_count = vertex_bone_count[vw.mVertexId]++;
						// save values for this bone move to next after ((++))

						assert(cur_vrtx_bone_count<BonedVertex::max_count_of_bones_per_vrtx);

						vertices[vw.mVertexId].bone_IDs[cur_vrtx_bone_count] = local_to_global_bone_ind.at(i);
						vertices[vw.mVertexId].bone_weights[cur_vrtx_bone_count] = vw.mWeight;
					}
				}

				return {vertices, indices};
			}

			void bind(const vk::raii::CommandBuffer& command_buffer) const
			{
				command_buffer.bindVertexBuffers(0, vertex_buffer_.getBuffer(), {0});

				command_buffer.bindIndexBuffer(index_buffer_.getBuffer(), 0, vk::IndexType::eUint16);
			}
		}* render_data_ = nullptr;
	};
}
