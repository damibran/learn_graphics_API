#pragma once
#include <glm/glm.hpp>
#include "Wrappers/Singletons/Singletons.h"
#include "Wrappers/Singletons/PerStaticModelData.h"
#include "Wrappers/BonedVertex.h"

namespace dmbrn
{
	/**
	 * \brief manages uniform GPU data for every skeletal model in scene
	 */
	class PerSkeletonData
	{
	public:
		uint32_t dynamic_aligned_size_ = 256;

		static inline constexpr size_t MAX_OBJECT_COUNT = 512;

		struct UBODynamicData
		{
			glm::mat4 model[BonedVertex::max_count_of_bones];
		};

		PerSkeletonData(const LogicalDevice& device, const PhysicalDevice& physical_device)
		{
			const uint32_t minUboAlignment = static_cast<uint32_t>(physical_device->getProperties().limits.minUniformBufferOffsetAlignment);
			if (minUboAlignment > 0)
			{
				constexpr uint32_t dynamicAlignment = sizeof(UBODynamicData);
				dynamic_aligned_size_ = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
			}

			const vk::DeviceSize bufferSize = MAX_OBJECT_COUNT * dynamic_aligned_size_;
			for (uint32_t i = 0; i < device.MAX_FRAMES_IN_FLIGHT; i++)
			{
				uniform_buffers_.push_back(
					device->createBuffer(
						vk::BufferCreateInfo{
							{}, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer
						}
					));

				uniform_buffers_memory.push_back(
					device->allocateMemory(
						vk::MemoryAllocateInfo{
							uniform_buffers_[i].getMemoryRequirements().size,
							physical_device.findMemoryType(uniform_buffers_[i].getMemoryRequirements().memoryTypeBits,
							                               vk::MemoryPropertyFlagBits::eHostVisible |
							                               vk::MemoryPropertyFlagBits::eHostCoherent)
						}
					));

				uniform_buffers_[i].bindMemory(*uniform_buffers_memory[i], 0);
			}

			createDescriptorSets(device);
		}

		char* map(uint32_t frame)
		{
			if (current_obj_count == 0)
				return nullptr;
			return static_cast<char*>(uniform_buffers_memory[frame].mapMemory(
				0, current_obj_count * dynamic_aligned_size_));
		}

		void unMap(uint32_t frame)
		{
			uniform_buffers_memory[frame].unmapMemory();
		}

		void bindDataFor(int frame, const vk::raii::CommandBuffer& command_buffer, vk::PipelineLayout layout,
		                 uint32_t offset) const
		{
			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			                                  layout, 3,
			                                  *descriptor_sets_[frame], offset);
		}

		uint32_t registerObject()
		{
			const uint32_t res = current_obj_offset_handle;
			current_obj_count++;
			current_obj_offset_handle += dynamic_aligned_size_;
			return res;
		}

		static vk::raii::DescriptorSetLayout createDescriptorLayout(const LogicalDevice& device)
		{
			const vk::DescriptorSetLayoutBinding FinalBoneMtxs
			{
				0, vk::DescriptorType::eUniformBufferDynamic,
				1, vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex
			};

			std::array<vk::DescriptorSetLayoutBinding, 1> bindings = {FinalBoneMtxs};

			const vk::DescriptorSetLayoutCreateInfo layoutInfo
			{
				{},
				bindings
			};

			return vk::raii::DescriptorSetLayout{device->createDescriptorSetLayout(layoutInfo)};
		}

		static inline vk::raii::DescriptorSetLayout descriptor_layout_{
			createDescriptorLayout(Singletons::device)
		};

	private:
		uint32_t current_obj_offset_handle = 0;
		uint32_t current_obj_count = 0;
		std::vector<vk::raii::Buffer> uniform_buffers_;
		std::vector<vk::raii::DeviceMemory> uniform_buffers_memory;
		std::vector<vk::raii::DescriptorSet> descriptor_sets_;

		void createDescriptorSets(const LogicalDevice& device)
		{
			std::vector<vk::DescriptorSetLayout> layouts(device.MAX_FRAMES_IN_FLIGHT, *descriptor_layout_);

			const vk::DescriptorSetAllocateInfo allocInfo
			{
				**Singletons::descriptor_pool,
				static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT),
				layouts.data()
			};

			descriptor_sets_ = device->allocateDescriptorSets(allocInfo);

			for (uint32_t i = 0; i < device.MAX_FRAMES_IN_FLIGHT; i++)
			{
				const vk::DescriptorBufferInfo FinalBoneMtxs
				{
					*uniform_buffers_[i], 0, sizeof(UBODynamicData)
				};

				std::array<vk::WriteDescriptorSet, 1> descriptorWrites{};

				descriptorWrites[0] = vk::WriteDescriptorSet
				{
					*descriptor_sets_[i], 0, 0, vk::DescriptorType::eUniformBufferDynamic,
					{}, FinalBoneMtxs
				};

				device->updateDescriptorSets(descriptorWrites, nullptr);
			}
		}
	};
}
