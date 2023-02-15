#pragma once

#include <string>
#include <vector>

#include <assimp/material.h>
#include <assimp/mesh.h>
#include<glm/glm.hpp>
#include"MaterialSystem/Materials/Diffusion/DiffusionMaterial.h"
#include "Texture.h"
#include "Vertex.h"

namespace std
{
	template <>
	struct hash<aiVector3D>
	{
		size_t operator()(const aiVector3D& vec) const noexcept
		{
			std::hash<float> hasher;
			size_t res = 0;
			res = hasher(vec.x) ^ hasher(vec.y) ^ hasher(vec.z);

			return res;
		}
	};

	template <>
	struct hash<std::vector<aiVector3D>>
	{
		size_t operator()(const std::vector<aiVector3D>& vertices) const noexcept
		{
			std::hash<aiVector3D> hasher;
			size_t res = 0;
			for (const auto& vec : vertices)
			{
				res ^= hasher(vec);
			}
			return res;
		}
	};

	template <>
	struct equal_to<std::vector<aiVector3D>>
	{
		bool operator()(const std::vector<aiVector3D>& lhs, const std::vector<aiVector3D>& rhs) const
		{
			if (lhs.size() != rhs.size())
				return false;

			bool same = true;
			for (size_t i = 0; i < lhs.size() && same; ++i)
			{
				if (lhs[i] != rhs[i])
					same = false;
			}

			return same;
		}
	};
}

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

		private:
			static inline std::unordered_map<std::vector<aiVector3D>, MeshRenderData> registry_;
			uint32_t indices_count;
			std::vector<std::string> use_this_mesh_;
			vk::raii::Buffer vertex_buffer_;
			vk::raii::Buffer index_buffer_;
			vk::raii::DeviceMemory vertex_buffer_memory_;
			vk::raii::DeviceMemory index_buffer_memory_;

			MeshRenderData(const std::string& mesh_name,const aiMesh* mesh):
				use_this_mesh_({mesh_name}),
				vertex_buffer_(nullptr),
				index_buffer_(nullptr),
				vertex_buffer_memory_(nullptr),
				index_buffer_memory_(nullptr)
			{
				auto [vertices,indices] = getDataFromMesh(mesh);
				indices_count = indices.size();

				createVertexBuffer(vertices, Singletons::physical_device, Singletons::device, Singletons::command_pool,
				                   Singletons::graphics_queue);
				createIndexBuffer(indices, Singletons::physical_device, Singletons::device, Singletons::command_pool,
				                  Singletons::graphics_queue);
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

			// initializes all the buffer objects/arrays
			void createVertexBuffer(const std::vector<Vertex>& vertices, const PhysicalDevice& physical_device,
			                        const LogicalDevice& device,
			                        const CommandPool& command_pool, const vk::raii::Queue& gragraphics_queue)
			{
				const vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

				vk::BufferCreateInfo bufferInfo
				{
					{}, bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
					vk::SharingMode::eExclusive
				};
				vk::raii::Buffer stagingBuffer = device->createBuffer(bufferInfo);

				const vk::MemoryAllocateInfo memory_allocate_info
				{
					stagingBuffer.getMemoryRequirements().size,
					physical_device.findMemoryType(stagingBuffer.getMemoryRequirements().memoryTypeBits,
					                               vk::MemoryPropertyFlagBits::eHostVisible |
					                               vk::MemoryPropertyFlagBits::eHostCoherent)
				};

				const vk::raii::DeviceMemory stagingBufferMemory = device->allocateMemory(memory_allocate_info);

				stagingBuffer.bindMemory(*stagingBufferMemory, 0);

				void* data;
				data = stagingBufferMemory.mapMemory(0, bufferSize, {});
				memcpy(data, vertices.data(), bufferSize);
				stagingBufferMemory.unmapMemory();

				bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
				vertex_buffer_ = vk::raii::Buffer{device->createBuffer(bufferInfo)};

				const auto allocate_info = vk::MemoryAllocateInfo
				{
					vertex_buffer_.getMemoryRequirements().size,
					physical_device.findMemoryType(vertex_buffer_.getMemoryRequirements().memoryTypeBits,
					                               vk::MemoryPropertyFlagBits::eDeviceLocal)
				};

				vertex_buffer_memory_ = vk::raii::DeviceMemory{device->allocateMemory(allocate_info)};

				vertex_buffer_.bindMemory(*vertex_buffer_memory_, 0);

				copyBuffer(device, command_pool, gragraphics_queue, stagingBuffer, vertex_buffer_, bufferSize);
			}

			void createIndexBuffer(const std::vector<uint16_t>& indices, const PhysicalDevice& physical_device,
			                       const LogicalDevice& device,
			                       const CommandPool& command_pool, vk::raii::Queue gragraphics_queue)
			{
				const vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

				vk::BufferCreateInfo bufferInfo
				{
					{}, bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
					vk::SharingMode::eExclusive
				};
				vk::raii::Buffer stagingBuffer = device->createBuffer(bufferInfo);

				const vk::MemoryAllocateInfo memory_allocate_info
				{
					stagingBuffer.getMemoryRequirements().size,
					physical_device.findMemoryType(stagingBuffer.getMemoryRequirements().memoryTypeBits,
					                               vk::MemoryPropertyFlagBits::eHostVisible |
					                               vk::MemoryPropertyFlagBits::eHostCoherent)
				};

				const vk::raii::DeviceMemory stagingBufferMemory = device->allocateMemory(memory_allocate_info);

				stagingBuffer.bindMemory(*stagingBufferMemory, 0);

				void* data = stagingBufferMemory.mapMemory(0, bufferSize, {});
				memcpy(data, indices.data(), bufferSize);
				stagingBufferMemory.unmapMemory();

				bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
				index_buffer_ = vk::raii::Buffer{device->createBuffer(bufferInfo)};

				const vk::MemoryAllocateInfo allocate_info
				{
					index_buffer_.getMemoryRequirements().size,
					physical_device.findMemoryType(index_buffer_.getMemoryRequirements().memoryTypeBits,
					                               vk::MemoryPropertyFlagBits::eDeviceLocal)
				};
				index_buffer_memory_ = vk::raii::DeviceMemory{device->allocateMemory(allocate_info)};

				index_buffer_.bindMemory(*index_buffer_memory_, 0);

				copyBuffer(device, command_pool, gragraphics_queue, stagingBuffer, index_buffer_, bufferSize);
			}

			void copyBuffer(const LogicalDevice& device, const CommandPool& command_pool,
			                vk::raii::Queue gragraphics_queue,
			                vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size)
			{
				vk::raii::CommandBuffer commandBuffer = command_pool.beginSingleTimeCommands(device);

				const vk::BufferCopy copyRegion
				{
					0, 0, size
				};
				commandBuffer.copyBuffer(*srcBuffer, *dstBuffer, copyRegion);

				command_pool.endSingleTimeCommands(gragraphics_queue, commandBuffer);
			}

			void bind(const vk::raii::CommandBuffer& command_buffer) const
			{
				command_buffer.bindVertexBuffers(0, *vertex_buffer_, {0});

				command_buffer.bindIndexBuffer(*index_buffer_, 0, vk::IndexType::eUint16);
			}
		}* render_data_=nullptr;
	};
}
