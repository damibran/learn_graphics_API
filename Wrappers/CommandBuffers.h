#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "LogicalDevice.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "GraphicsPipeline.h"
#include "CommandPool.h"
#include "Model.h"
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

		void recordCommandBuffer(const LogicalDevice& device, const RenderPass& render_pass, const GraphicsPipeline& graphics_pipeline,
			const SwapChain& swap_chain, const Model& model,
			const DescriptorSets& descriptor_sets,
			int currentFrame, uint32_t imageIndex)const
		{
			const vk::CommandBufferBeginInfo beginInfo{};

			const vk::raii::CommandBuffer& command_buffer = command_buffers_[currentFrame];

			command_buffer.begin(beginInfo);

			const std::array<vk::ClearValue, 2> clear_values
			{
				vk::ClearValue{ vk::ClearColorValue{ std::array<float,4>{0.0f, 0.0f, 0.0f, 1.0f} } },
				vk::ClearValue{vk::ClearDepthStencilValue{1.0f, 0}}
			};

			const vk::RenderPassBeginInfo renderPassInfo
			{
				**render_pass,
				*swap_chain.getFrameBuffers()[imageIndex],
				vk::Rect2D{vk::Offset2D{ 0, 0 }, swap_chain.getExtent()},
				clear_values
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

			//model drawing
			model.Draw(currentFrame, device, graphics_pipeline, command_buffer, descriptor_sets);

			command_buffer.endRenderPass();

			command_buffer.end();
		}

		const vk::raii::CommandBuffer& operator[](int index)const
		{
			return command_buffers_[index];
		}

	private:
		std::vector<vk::raii::CommandBuffer> command_buffers_;
	};
}
