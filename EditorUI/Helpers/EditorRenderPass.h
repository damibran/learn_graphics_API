#pragma once

#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Wrappers/Singletons/Surface.h"
#include "Wrappers/Singletons/PhysicalDevice.h"
#include "Wrappers/Singletons/LogicalDevice.h"
#include "Utils/UtilsFunctions.h"

namespace dmbrn
{
	/**
	 * \brief represents render pass of editor UI
	 */
	class EditorRenderPass
	{
	public:
		EditorRenderPass():
			render_pass_(nullptr)
		{
			const vk::AttachmentDescription colorAttachment
			{
				{},
				utils::chooseSwapSurfaceFormat(PhysicalDevice::querySurfaceFormats(*Singletons::physical_device, Singletons::surface)).format,
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
				0, vk::ImageLayout::eAttachmentOptimal
			};

			const vk::SubpassDescription subpass
			{
				{},
				vk::PipelineBindPoint::eGraphics,
				nullptr,
				colorAttachmentRef
			};

			const vk::SubpassDependency dependency
			{
				VK_SUBPASS_EXTERNAL, 0,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::AccessFlagBits::eNone,
				vk::AccessFlagBits::eColorAttachmentWrite
			};

			const std::array<vk::AttachmentDescription, 1> attachments{colorAttachment};

			const vk::RenderPassCreateInfo renderPassInfo
			{
				{},
				attachments,
				subpass,
				dependency
			};

			render_pass_ = vk::raii::RenderPass{Singletons::device->createRenderPass(renderPassInfo)};
		}

		const vk::raii::RenderPass& operator*() const
		{
			return render_pass_;
		}

		const vk::raii::RenderPass* operator->() const
		{
			return &render_pass_;
		}

	private:
		vk::raii::RenderPass render_pass_;
	};
}
