#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <fstream>
#include <optional>
#include <set>

#include "GLFWwindowWrapper.h"
#include "Instance.h"
#include "Surface.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "../Utils//UtilsFunctions.h"

namespace dmbrn
{
	class RenderPass
	{
	public:
		RenderPass(const Surface& surface, const PhysicalDevice& physical_device, const LogicalDevice& device)
		{
			vk::AttachmentDescription colorAttachment{};
			colorAttachment.format = utils::chooseSwapSurfaceFormat(PhysicalDevice::querySurfaceFormats(*physical_device, surface)).format;
			colorAttachment.samples = vk::SampleCountFlagBits::e1;
			colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
			colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
			colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
			colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

			vk::AttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = vk::ImageLayout::eAttachmentOptimal;

			vk::SubpassDescription subpass{};
			subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			vk::SubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			dependency.srcAccessMask = vk::AccessFlagBits::eNone;
			dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

			vk::RenderPassCreateInfo renderPassInfo{};
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			render_pass_ = std::make_unique<vk::raii::RenderPass>(device->createRenderPass(renderPassInfo));
		}

		const vk::raii::RenderPass& operator*()const
		{
			return *render_pass_;
		}

		vk::raii::RenderPass* operator->()const
		{
			return render_pass_.get();
		}

	private:
		std::unique_ptr<vk::raii::RenderPass> render_pass_;

	};
}