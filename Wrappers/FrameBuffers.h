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

#include "GLFWwindowWrapper.h"
#include "Instance.h"
#include "Surface.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "DescriptorSetLayout.h"
#include "GraphicsPipeline.h"

namespace dmbrn
{
	class FrameBuffers
	{
	public:
		FrameBuffers(const LogicalDevice& device, const SwapChain& swap_chain,const RenderPass& render_pass)
		{
			for (size_t i = 0; i < swap_chain.getImageViews().size(); i++)
			{
				vk::ImageView attachments[] = {
					*swap_chain.getImageViews()[i]
				};

				vk::FramebufferCreateInfo framebufferInfo{};
				framebufferInfo.renderPass = **render_pass;
				framebufferInfo.attachmentCount = 1;
				framebufferInfo.pAttachments = attachments;
				framebufferInfo.width = swap_chain.getExtent().width;
				framebufferInfo.height = swap_chain.getExtent().height;
				framebufferInfo.layers = 1;

				framebuffers_.push_back(device->createFramebuffer(framebufferInfo));
			}
		}
	private:
		std::vector<vk::raii::Framebuffer> framebuffers_;
	};
}
