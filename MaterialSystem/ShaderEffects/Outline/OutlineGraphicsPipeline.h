#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <fstream>

#include "Wrappers/Singletons/LogicalDevice.h"
#include "Wrappers/Vertex.h"
#include "EditorUI/Viewport/ViewportRenderPass.h"

namespace dmbrn
{
	class OutlineGraphicsPipeline
	{
	public:

		static vk::raii::Pipeline createStaticPipeline(const LogicalDevice& device, const vk::raii::RenderPass& render_pass,
		                   const vk::raii::PipelineLayout& pipeline_layout)
		{
			const auto vertShaderCode = readFile("shaders/outline_vert.spv");
			const auto fragShaderCode = readFile("shaders/outline_frag.spv");

			const vk::raii::ShaderModule vertShaderModule = createShaderModule(device, vertShaderCode);
			const vk::raii::ShaderModule fragShaderModule = createShaderModule(device, fragShaderCode);

			const vk::PipelineShaderStageCreateInfo vertShaderStageInfo
			{
				{}, vk::ShaderStageFlagBits::eVertex,
				*vertShaderModule, "main"
			};

			const vk::PipelineShaderStageCreateInfo fragShaderStageInfo
			{
				{}, vk::ShaderStageFlagBits::eFragment,
				*fragShaderModule, "main"
			};

			const vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};


			const auto bindingDescription = Vertex::getBindingDescription();
			const auto attributeDescriptions = Vertex::getAttributeDescriptions()[0];

			const vk::PipelineVertexInputStateCreateInfo vertexInputInfo
			{
				{},
				bindingDescription, attributeDescriptions
			};

			// SAME ////////////////////////////////////////////////////

			const vk::PipelineInputAssemblyStateCreateInfo inputAssembly
			{
				{}, vk::PrimitiveTopology::eTriangleList,VK_FALSE
			};

			const vk::PipelineViewportStateCreateInfo viewportState
			{
				{}, 1, {}, 1, {}
			};

			const vk::PipelineRasterizationStateCreateInfo rasterizer
			{
				{}, VK_FALSE,
				VK_FALSE, vk::PolygonMode::eFill,
				vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
				VK_FALSE, {}, {}, {}, 1.0f
			};

			const vk::PipelineMultisampleStateCreateInfo multisampling
			{
				{}, vk::SampleCountFlagBits::e1,VK_FALSE
			};

			const vk::PipelineColorBlendAttachmentState colorBlendAttachment
			{
				VK_FALSE, {}, {}, {}, {}, {}, {}, // not like dis
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA
			};

			const vk::PipelineColorBlendStateCreateInfo colorBlending
			{
				{},VK_FALSE, vk::LogicOp::eCopy,
				1, &colorBlendAttachment, {0.0f, 0.0f, 0.0f, 0.0f}
			};

			const std::vector dynamicStates
			{
				vk::DynamicState::eViewport,
				vk::DynamicState::eScissor,
			};

			const vk::PipelineDynamicStateCreateInfo dynamicState
			{
				{}, static_cast<uint32_t>(dynamicStates.size()),
				dynamicStates.data()
			};

			const vk::StencilOpState stencil_op
			{
				vk::StencilOp::eKeep, vk::StencilOp::eReplace, vk::StencilOp::eKeep, vk::CompareOp::eNotEqual, 0xff,
				0xff, 1

			};

			const vk::PipelineDepthStencilStateCreateInfo depth_stencil_info
			{
				{}, VK_FALSE, VK_FALSE, vk::CompareOp::eLessOrEqual,
				VK_FALSE, VK_TRUE, stencil_op, stencil_op
			};

			const vk::GraphicsPipelineCreateInfo pipelineInfo
			{
				{}, 2, shaderStages,
				&vertexInputInfo, &inputAssembly, {},
				&viewportState, &rasterizer, &multisampling,
				&depth_stencil_info, &colorBlending, &dynamicState,
				*pipeline_layout, *render_pass
			};

			return vk::raii::Pipeline{device->createGraphicsPipeline(nullptr, pipelineInfo)};
		}

		static vk::raii::Pipeline createSkeletalPipeline(const LogicalDevice& device, const vk::raii::RenderPass& render_pass,
		                   const vk::raii::PipelineLayout& pipeline_layout)
		{
			const auto vertShaderCode = readFile("shaders/outline_skeletal_vert.spv");
			const auto fragShaderCode = readFile("shaders/outline_frag.spv");

			const vk::raii::ShaderModule vertShaderModule = createShaderModule(device, vertShaderCode);
			const vk::raii::ShaderModule fragShaderModule = createShaderModule(device, fragShaderCode);

			const vk::PipelineShaderStageCreateInfo vertShaderStageInfo
			{
				{}, vk::ShaderStageFlagBits::eVertex,
				*vertShaderModule, "main"
			};

			const vk::PipelineShaderStageCreateInfo fragShaderStageInfo
			{
				{}, vk::ShaderStageFlagBits::eFragment,
				*fragShaderModule, "main"
			};

			const vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};


			const auto bindingDescription = BonedVertex::getBindingDescription();
			const auto attributeDescriptions = BonedVertex::getAttributeDescriptions();

			const vk::PipelineVertexInputStateCreateInfo vertexInputInfo
			{
				{},
				bindingDescription, attributeDescriptions
			};

			// SAME ////////////////////////////////////////////////////

			const vk::PipelineInputAssemblyStateCreateInfo inputAssembly
			{
				{}, vk::PrimitiveTopology::eTriangleList,VK_FALSE
			};

			const vk::PipelineViewportStateCreateInfo viewportState
			{
				{}, 1, {}, 1, {}
			};

			const vk::PipelineRasterizationStateCreateInfo rasterizer
			{
				{}, VK_FALSE,
				VK_FALSE, vk::PolygonMode::eFill,
				vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
				VK_FALSE, {}, {}, {}, 1.0f
			};

			const vk::PipelineMultisampleStateCreateInfo multisampling
			{
				{}, vk::SampleCountFlagBits::e1,VK_FALSE
			};

			const vk::PipelineColorBlendAttachmentState colorBlendAttachment
			{
				VK_FALSE, {}, {}, {}, {}, {}, {}, // not like dis
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA
			};

			const vk::PipelineColorBlendStateCreateInfo colorBlending
			{
				{},VK_FALSE, vk::LogicOp::eCopy,
				1, &colorBlendAttachment, {0.0f, 0.0f, 0.0f, 0.0f}
			};

			const std::vector dynamicStates
			{
				vk::DynamicState::eViewport,
				vk::DynamicState::eScissor,
			};

			const vk::PipelineDynamicStateCreateInfo dynamicState
			{
				{}, static_cast<uint32_t>(dynamicStates.size()),
				dynamicStates.data()
			};

			const vk::StencilOpState stencil_op
			{
				vk::StencilOp::eKeep, vk::StencilOp::eReplace, vk::StencilOp::eKeep, vk::CompareOp::eNotEqual, 0xff,
				0xff, 1

			};

			const vk::PipelineDepthStencilStateCreateInfo depth_stencil_info
			{
				{}, VK_FALSE, VK_FALSE, vk::CompareOp::eLessOrEqual,
				VK_FALSE, VK_TRUE, stencil_op, stencil_op
			};

			const vk::GraphicsPipelineCreateInfo pipelineInfo
			{
				{}, 2, shaderStages,
				&vertexInputInfo, &inputAssembly, {},
				&viewportState, &rasterizer, &multisampling,
				&depth_stencil_info, &colorBlending, &dynamicState,
				*pipeline_layout, *render_pass
			};

			return vk::raii::Pipeline{device->createGraphicsPipeline(nullptr, pipelineInfo)};
		}
	private:

		static std::vector<char> readFile(const std::string& filename)
		{
			std::ifstream file(filename, std::ios::ate | std::ios::binary);

			if (!file.is_open())
			{
				throw std::runtime_error("failed to open " + filename + " file!");
			}

			const size_t fileSize = file.tellg();
			std::vector<char> buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);

			file.close();

			return buffer;
		}

		static vk::raii::ShaderModule createShaderModule(const LogicalDevice& device, const std::vector<char>& code)
		{
			const vk::ShaderModuleCreateInfo createInfo
			{
				{}, code.size(),
				reinterpret_cast<const uint32_t*>(code.data())
			};
			return device->createShaderModule(createInfo);
		}
	};
}
