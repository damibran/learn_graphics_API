#pragma once
#include <Wrappers/Singletons/Singletons.h>
#include "CameraUniformBuffer.h"

namespace dmbrn
{
	class CameraRenderData
	{
	public:
		CameraRenderData():
			uniform_buffers_{Singletons::physical_device, Singletons::device}
		{
			createDescriptorSets(Singletons::device);
		}

		void update(int frame, const glm::mat4& view, const glm::mat4& proj)
		{
			auto data = static_cast<CameraUniformBuffer::UniformBufferObject*>(uniform_buffers_.getUBMemory(frame).
				mapMemory(0, sizeof(CameraUniformBuffer::UniformBufferObject)));
			data->view = view;
			data->proj = proj;
			uniform_buffers_.getUBMemory(frame).unmapMemory();
		}

		void bind(int frame, const vk::raii::CommandBuffer& command_buffer)const
		{

			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			                                  *pipeline_layout_, 0,
			                                  *descriptor_sets_[frame], nullptr);
		}

		static vk::raii::DescriptorSetLayout& getDescriptorSetLayout()
		{
			return descriptor_set_layout_;
		}

	private:
		CameraUniformBuffer uniform_buffers_;
		std::vector<vk::raii::DescriptorSet> descriptor_sets_;

		void createDescriptorSets(const LogicalDevice& device)
		{
			std::vector<vk::DescriptorSetLayout> layouts(device.MAX_FRAMES_IN_FLIGHT, *descriptor_set_layout_);

			const vk::DescriptorSetAllocateInfo allocInfo
			{
				**Singletons::descriptor_pool,
				static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT),
				layouts.data()
			};

			descriptor_sets_ = device->allocateDescriptorSets(allocInfo);

			for (uint32_t i = 0; i < device.MAX_FRAMES_IN_FLIGHT; i++)
			{
				vk::DescriptorBufferInfo bufferInfo
				{
					*uniform_buffers_[i], 0, sizeof(CameraUniformBuffer::UniformBufferObject)
				};


				std::array<vk::WriteDescriptorSet, 1> descriptorWrites{};

				descriptorWrites[0] = vk::WriteDescriptorSet
				{
					*descriptor_sets_[i], 0, 0, vk::DescriptorType::eUniformBuffer,
					{}, bufferInfo
				};

				device->updateDescriptorSets(descriptorWrites, nullptr);
			}
		}

		static vk::raii::DescriptorSetLayout createDescriptorLayout(const LogicalDevice& device)
		{
			const vk::DescriptorSetLayoutBinding uboLayoutBinding
			{
				0, vk::DescriptorType::eUniformBuffer,
				1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
			};

			std::array<vk::DescriptorSetLayoutBinding, 1> bindings = {uboLayoutBinding};

			const vk::DescriptorSetLayoutCreateInfo layoutInfo
			{
				{},
				bindings
			};

			return vk::raii::DescriptorSetLayout{device->createDescriptorSetLayout(layoutInfo)};
		}

		static inline vk::raii::DescriptorSetLayout descriptor_set_layout_{createDescriptorLayout(Singletons::device)};

		static vk::raii::PipelineLayout createPipelineLayout()
		{
			const vk::PushConstantRange push_constant_range
			{
				vk::ShaderStageFlagBits::eVertex,0,sizeof(glm::mat4)
			};

			const vk::PipelineLayoutCreateInfo info
			{
				{}, *descriptor_set_layout_, push_constant_range
			};

			return Singletons::device->createPipelineLayout(info);
		}

		static inline vk::raii::PipelineLayout pipeline_layout_{createPipelineLayout()};
	};
}
