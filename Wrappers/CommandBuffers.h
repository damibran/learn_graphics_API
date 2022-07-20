#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <fstream>
#include <optional>
#include <set>

#include <glm/glm.hpp>
#include <stb_image.h>

#include "GLFWwindowWrapper.h"
#include "Instance.h"
#include "Surface.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "DescriptorSetLayout.h"
#include "GraphicsPipeline.h"
#include "CommandPool.h"
#include "Texture.h"
#include "VertexIndexBuffers.h"
#include "UniformBuffers.h"
#include "DescriptorSets.h"

namespace dmbrn
{
	class CommandBuffers
	{
	public:
		CommandBuffers(const LogicalDevice& device, const CommandPool& command_pool)
		{
			vk::CommandBufferAllocateInfo allocInfo{};
			allocInfo.commandPool = **command_pool;
			allocInfo.level = vk::CommandBufferLevel::ePrimary;
			allocInfo.commandBufferCount = (uint32_t)device.MAX_FRAMES_IN_FLIGHT;

			command_buffers_ = device->allocateCommandBuffers(allocInfo);
		}

		void recordCommandBuffer(const RenderPass& render_pass, const GraphicsPipeline& graphics_pipeline,
			const SwapChain& swap_chain, const VertexIndexBuffers& vertex_index_buffers,
			const DescriptorSets& descriptor_sets,
			 int currentFrame, uint32_t imageIndex)
		{
			vk::CommandBufferBeginInfo beginInfo{};

			vk::raii::CommandBuffer& command_buffer = command_buffers_[currentFrame];

			command_buffer.begin(beginInfo);

			vk::RenderPassBeginInfo renderPassInfo{};
			renderPassInfo.renderPass = **render_pass;
			renderPassInfo.framebuffer = *swap_chain.getFrameBuffers()[imageIndex];
			renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
			renderPassInfo.renderArea.extent = swap_chain.getExtent();

			vk::ClearValue clearColor = vk::ClearValue{vk::ClearColorValue{ std::array<float,4>{0.0f, 0.0f, 0.0f, 1.0f} }};
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			command_buffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

			command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **graphics_pipeline);

			vk::Viewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)swap_chain.getExtent().width;
			viewport.height = (float)swap_chain.getExtent().height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			command_buffer.setViewport(0, viewport);

			vk::Rect2D scissor{};
			scissor.offset = vk::Offset2D{ 0, 0 };
			scissor.extent = swap_chain.getExtent();
			command_buffer.setScissor(0, scissor);

			vk::Buffer vertexBuffers[] = { *vertex_index_buffers.getVertex() };
			vk::DeviceSize offsets[] = { 0 };
			command_buffer.bindVertexBuffers(0, vertexBuffers, offsets);

			command_buffer.bindIndexBuffer(*vertex_index_buffers.getIndex(), 0, vk::IndexType::eUint16);

			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *graphics_pipeline.getLayout(), 0,
				*descriptor_sets[currentFrame], nullptr);

			command_buffer.drawIndexed(vertex_index_buffers.getIndeciesCount(), 1, 0, 0, 0);

			command_buffer.endRenderPass();

			command_buffer.end();
		}

		const vk::raii::CommandBuffer& operator[](int index)
		{
			return command_buffers_[index];
		}

	private:
		std::vector<vk::raii::CommandBuffer> command_buffers_;
	};
}
