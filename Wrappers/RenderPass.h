#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

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
			const vk::AttachmentDescription colorAttachment
			{
				{},
				utils::chooseSwapSurfaceFormat(PhysicalDevice::querySurfaceFormats(*physical_device, surface)).format,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::ePresentSrcKHR
			};

			const vk::AttachmentReference colorAttachmentRef
			{
				0,vk::ImageLayout::eAttachmentOptimal
			};

			const vk::SubpassDescription subpass
			{
				{},
				vk::PipelineBindPoint::eGraphics,
				0,{},
				1, &colorAttachmentRef
			};

			const vk::SubpassDependency dependency
			{
				VK_SUBPASS_EXTERNAL, 0,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::AccessFlagBits::eNone,
				vk::AccessFlagBits::eColorAttachmentWrite
			};

			const vk::RenderPassCreateInfo renderPassInfo
			{
				{},
				colorAttachment,
				subpass,
				dependency
			};

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