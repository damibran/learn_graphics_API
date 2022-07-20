#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <fstream>

#include "LogicalDevice.h"
#include "RenderPass.h"
#include "DescriptorSetLayout.h"
#include "Vertex.h"

namespace dmbrn
{
	class GraphicsPipeline
	{
	public:
		GraphicsPipeline(const LogicalDevice& device, const RenderPass& render_pass, const DescriptorSetLayout& descriptor_set_layout)
		{
			auto vertShaderCode = readFile("shaders/vert.spv");
			auto fragShaderCode = readFile("shaders/frag.spv");

			vk::raii::ShaderModule vertShaderModule = createShaderModule(device, vertShaderCode);
			vk::raii::ShaderModule fragShaderModule = createShaderModule(device, fragShaderCode);

			vk::PipelineShaderStageCreateInfo vertShaderStageInfo
			{
				{}, vk::ShaderStageFlagBits::eVertex,
				*vertShaderModule, "main"
			};

			vk::PipelineShaderStageCreateInfo fragShaderStageInfo
			{
				{}, vk::ShaderStageFlagBits::eFragment,
				*fragShaderModule, "main"
			};

			vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };


			auto bindingDescription = Vertex::getBindingDescription();
			auto attributeDescriptions = Vertex::getAttributeDescriptions();

			vk::PipelineVertexInputStateCreateInfo vertexInputInfo
			{
				{},
				bindingDescription,  attributeDescriptions
			};

			vk::PipelineInputAssemblyStateCreateInfo inputAssembly
			{
				{},vk::PrimitiveTopology::eTriangleList,VK_FALSE
			};

			vk::PipelineViewportStateCreateInfo viewportState
			{
				{}, 1,{},1,{}
			};

			vk::PipelineRasterizationStateCreateInfo rasterizer
			{
				{}, VK_FALSE,
				VK_FALSE, vk::PolygonMode::eFill,
				vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
				VK_FALSE, {}, {}, {}, 1.0f
			};

			vk::PipelineMultisampleStateCreateInfo multisampling
			{
				{},vk::SampleCountFlagBits::e1,VK_FALSE
			};

			vk::PipelineColorBlendAttachmentState colorBlendAttachment
			{
				VK_FALSE,{},{},{},{},{},{}, // not like dis
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA
			};

			vk::PipelineColorBlendStateCreateInfo colorBlending
			{
				{},VK_FALSE,vk::LogicOp::eCopy,
				1, &colorBlendAttachment, {0.0f,0.0f,0.0f,0.0f}
			};

			std::vector dynamicStates
			{
				vk::DynamicState::eViewport,
				vk::DynamicState::eScissor
			};

			vk::PipelineDynamicStateCreateInfo dynamicState
			{
				{}, static_cast<uint32_t>(dynamicStates.size()),
				dynamicStates.data()
			};

			vk::PipelineLayoutCreateInfo pipelineLayoutInfo
			{
				{}, 1, &**descriptor_set_layout
			};

			pipeline_layout_ = std::make_unique<vk::raii::PipelineLayout>(device->createPipelineLayout(pipelineLayoutInfo));

			vk::GraphicsPipelineCreateInfo pipelineInfo
			{
				{}, 2, shaderStages,
				&vertexInputInfo,&inputAssembly,{},
				&viewportState,&rasterizer,&multisampling,
				{},&colorBlending,&dynamicState,
				**pipeline_layout_,**render_pass
			};

			graphics_pipeline_ = std::make_unique<vk::raii::Pipeline>(device->createGraphicsPipeline(nullptr, pipelineInfo));
		}

		const vk::raii::Pipeline& operator*()const
		{
			return *graphics_pipeline_;
		}

		const vk::raii::PipelineLayout& getLayout()const
		{
			return *pipeline_layout_;
		}

	private:
		std::unique_ptr<vk::raii::PipelineLayout> pipeline_layout_;
		std::unique_ptr<vk::raii::Pipeline> graphics_pipeline_;

		static std::vector<char> readFile(const std::string& filename)
		{
			std::ifstream file(filename, std::ios::ate | std::ios::binary);

			if (!file.is_open()) {
				throw std::runtime_error("failed to open file!");
			}

			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);

			file.close();

			return buffer;
		}

		static vk::raii::ShaderModule createShaderModule(const LogicalDevice& device, const std::vector<char>& code)
		{
			vk::ShaderModuleCreateInfo createInfo
			{
				{},code.size(),
				reinterpret_cast<const uint32_t*>(code.data())
			};
			return device->createShaderModule(createInfo);
		}
	};
}
