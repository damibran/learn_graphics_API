#pragma once

#include <string>
#include <vector>

#include "Texture.h"
#include "Vertex.h"

namespace dmbrn
{
	glm::mat4 toGlmMat(const aiMatrix4x4& mat)
	{
		return glm::mat4
		{mat.a1,mat.a2,mat.a3,mat.a4,
			mat.b1,mat.b2,mat.b3,mat.b4,
			mat.c1,mat.c2,mat.c3,mat.c4,
			mat.d1,mat.d2,mat.d3,mat.d4,
		};
	}

	class Mesh
	{
	public:
		UnlitTexturedMaterial* material_;

		uint32_t indices_count;

		glm::mat4 transform_;

		Mesh(const Mesh& other) = delete;
		Mesh& operator=(const Mesh& other) = delete;
		~Mesh() = default;

		Mesh(Mesh&& other) = default;

		Mesh& operator=(Mesh&& other) = default;

		Mesh(const std::string& dir, const std::string& model_name, const aiMaterial* ai_material, aiMesh* mesh,
		     const aiMatrix4x4& transform):
			transform_(toGlmMat(transform)),
			vertex_buffer_(nullptr),
			index_buffer_(nullptr),
			vertex_buffer_memory_(nullptr),
			index_buffer_memory_(nullptr)
		{
			material_ = UnlitTexturedMaterial::GetMaterialPtr(dir, model_name, ai_material);

			auto [vertices,indices] = fillVectors(mesh);
			indices_count = indices.size();

			createVertexBuffer(vertices, Singletons::physical_device, Singletons::device, Singletons::command_pool,
			                   Singletons::graphics_queue);
			createIndexBuffer(indices, Singletons::physical_device, Singletons::device, Singletons::command_pool,
			                  Singletons::graphics_queue);
		}

		vk::raii::Buffer vertex_buffer_;
		vk::raii::Buffer index_buffer_;
	private:
		// render data
		vk::raii::DeviceMemory vertex_buffer_memory_;
		vk::raii::DeviceMemory index_buffer_memory_;
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

		void copyBuffer(const LogicalDevice& device, const CommandPool& command_pool, vk::raii::Queue gragraphics_queue,
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

		std::pair<std::vector<Vertex>, std::vector<uint16_t>> fillVectors(aiMesh* mesh)
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

			return {vertices, indices};
		}
	};
}
