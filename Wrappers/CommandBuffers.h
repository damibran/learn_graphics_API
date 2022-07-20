#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "LogicalDevice.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "GraphicsPipeline.h"
#include "CommandPool.h"
#include "VertexIndexBuffers.h"
#include "DescriptorSets.h"

namespace dmbrn
{
	class CommandBuffers
	{
	public:
		CommandBuffers(const LogicalDevice& device, const CommandPool& command_pool)
		{
			const vk::CommandBufferAllocateInfo allocInfo
			{
				**command_pool, vk::CommandBufferLevel::ePrimary,
				 static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT)
			};

			command_buffers_ = device->allocateCommandBuffers(allocInfo);
		}

		void recordCommandBuffer(const RenderPass& render_pass, const GraphicsPipeline& graphics_pipeline,
			const SwapChain& swap_chain, const VertexIndexBuffers& vertex_index_buffers,
			const DescriptorSets& descriptor_sets,
			int currentFrame, uint32_t imageIndex)const
		{
			const vk::CommandBufferBeginInfo beginInfo{};

			const vk::raii::CommandBuffer& command_buffer = command_buffers_[currentFrame];

			command_buffer.begin(beginInfo);

			vk::ClearValue clearColor = vk::ClearValue{ vk::ClearColorValue{ std::array<float,4>{0.0f, 0.0f, 0.0f, 1.0f} } };
			const vk::RenderPassBeginInfo renderPassInfo
			{
				**render_pass,
				*swap_chain.getFrameBuffers()[imageIndex],
				vk::Rect2D{vk::Offset2D{ 0, 0 }, swap_chain.getExtent()},
				clearColor
			};

			command_buffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

			command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **graphics_pipeline);

			const vk::Viewport viewport
			{
				0.0f,0.0f,
				static_cast<float>(swap_chain.getExtent().width),
				static_cast<float>(swap_chain.getExtent().height),
				0.0f, 1.0f
			};
			command_buffer.setViewport(0, viewport);

			const vk::Rect2D scissor
			{
				vk::Offset2D{ 0, 0 },
				swap_chain.getExtent()
			};
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
