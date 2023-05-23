#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Wrappers/Singletons/LogicalDevice.h"
#include "Utils/UtilsFunctions.h"

namespace dmbrn
{
	/**
	 * \brief  represents viewport render pass
	 */
	class ViewportRenderPass
	{
	public:
		ViewportRenderPass():
			render_pass_(nullptr)
		{
			constexpr vk::AttachmentDescription colorAttachment
			{
				{},
				vk::Format::eR8G8B8A8Srgb,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eShaderReadOnlyOptimal,
				vk::ImageLayout::eShaderReadOnlyOptimal
			};

			constexpr vk::AttachmentReference colorAttachmentRef
			{
				0, vk::ImageLayout::eShaderReadOnlyOptimal
			};

			const vk::AttachmentDescription depthAttachment
			{
				{},
				utils::findDepthFormat(Singletons::physical_device),
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eDontCare,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eDepthStencilAttachmentOptimal
			};

			constexpr vk::AttachmentReference depthAttachmentRef
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

			constexpr vk::SubpassDependency depthStencilDependency
			{
				VK_SUBPASS_EXTERNAL,0,
				vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
				vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
				 vk::AccessFlagBits::eDepthStencilAttachmentWrite,
				vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead,{}
			};

			constexpr vk::SubpassDependency colorDependency
			{
				VK_SUBPASS_EXTERNAL,
				0,
				vk::PipelineStageFlagBits::eFragmentShader,
				vk::PipelineStageFlagBits::eFragmentShader,
				vk::AccessFlagBits::eColorAttachmentWrite,
				vk::AccessFlagBits::eShaderRead
			};

			const std::array<vk::AttachmentDescription, 2> attachments{colorAttachment, depthAttachment};

			constexpr std::array<vk::SubpassDependency, 2> dependencies{colorDependency,depthStencilDependency};

			const vk::RenderPassCreateInfo renderPassInfo
			{
				{},
				attachments,
				subpass,
				dependencies
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
