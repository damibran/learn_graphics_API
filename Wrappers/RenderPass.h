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
		RenderPass(const Surface& surface, const PhysicalDevice& physical_device, const LogicalDevice& device):
			render_pass_(nullptr)
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

			const vk::AttachmentDescription depthAttachment
			{
				{},
				utils::findDepthFormat(physical_device),
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eDontCare,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eDepthStencilAttachmentOptimal
			};

			const vk::AttachmentReference depthAttachmentRef
			{
				1,vk::ImageLayout::eDepthStencilAttachmentOptimal
			};

			const vk::SubpassDescription subpass
			{
				{},
				vk::PipelineBindPoint::eGraphics,
				nullptr,
				colorAttachmentRef,
				{},
				&depthAttachmentRef
			};

			const vk::SubpassDependency dependency
			{
				VK_SUBPASS_EXTERNAL, 0,
				vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
				vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
				vk::AccessFlagBits::eNone,
				vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite
			};

			const std::array<vk::AttachmentDescription, 2> attachments{ colorAttachment,depthAttachment };

			const vk::RenderPassCreateInfo renderPassInfo
			{
				{},
				attachments,
				subpass,
				dependency
			};

			render_pass_ = vk::raii::RenderPass{device->createRenderPass(renderPassInfo)};
		}

		const vk::raii::RenderPass& operator*()const
		{
			return render_pass_;
		}

		const vk::raii::RenderPass* operator->()const
		{
			return &render_pass_;
		}

	private:
		vk::raii::RenderPass render_pass_;

	};
}