#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Wrappers/Singletons/Surface.h"
#include "Wrappers/Singletons/PhysicalDevice.h"
#include "Wrappers/Singletons/LogicalDevice.h"
#include "Utils/UtilsFunctions.h"

namespace dmbrn
{
	class ViewportRenderPass
	{
	public:
		ViewportRenderPass(const Surface& surface, const PhysicalDevice& physical_device, const LogicalDevice& device):
			render_pass_(nullptr)
		{
			const vk::AttachmentDescription colorAttachment
			{
				{},
				vk::Format::eR8G8B8A8Srgb,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eColorAttachmentOptimal,
				vk::ImageLayout::eShaderReadOnlyOptimal
			};

			const vk::AttachmentReference colorAttachmentRef
			{
				0, vk::ImageLayout::eColorAttachmentOptimal
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
				1, vk::ImageLayout::eDepthStencilAttachmentOptimal
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

			const std::array<vk::AttachmentDescription, 2> attachments{colorAttachment, depthAttachment};

			const vk::RenderPassCreateInfo renderPassInfo
			{
				{},
				attachments,
				subpass,
				dependency
			};

			render_pass_ = vk::raii::RenderPass{device->createRenderPass(renderPassInfo)};
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
