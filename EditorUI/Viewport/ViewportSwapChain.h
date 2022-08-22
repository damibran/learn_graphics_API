#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Wrappers/Singletons/LogicalDevice.h"
#include "Utils/UtilsFunctions.h"
#include "Wrappers/Depthbuffer.h"
#include "Wrappers/Singletons/Singletons.h"
#include "Wrappers/Texture.h"

namespace dmbrn
{
	class ViewportSwapChain
	{
	public:
		ViewportSwapChain(vk::Extent2D extent, const ViewportRenderPass& render_pass):
			extent_(extent),
			color_buffers_(createColorBuffers(extent_)),
			depth_buffer_(extent_, Singletons::physical_device,Singletons::device),
			framebuffers_(createFrameBuffers(Singletons::device, render_pass))
		{
		}

		void recreate(const vk::Extent2D extent, const ViewportRenderPass& render_pass)
		{
			extent_ = extent;

			Singletons::device->waitIdle(); // may be not

			color_buffers_ = createColorBuffers(extent_);
			depth_buffer_ = DepthBuffer(extent_, Singletons::physical_device,
			                            Singletons::device);
			framebuffers_ = createFrameBuffers(Singletons::device, render_pass);
		}

		const vk::Extent2D& getExtent() const
		{
			return extent_;
		}

		const std::vector<Texture>& getColorBuffers() const
		{
			return color_buffers_;
		}

		const std::vector<vk::raii::Framebuffer>& getFrameBuffers() const
		{
			return framebuffers_;
		}

	private:
		vk::Extent2D extent_;
		std::vector<Texture> color_buffers_;
		DepthBuffer depth_buffer_;
		std::vector<vk::raii::Framebuffer> framebuffers_;

		[[nodiscard]] std::vector<Texture> createColorBuffers(vk::Extent2D extent)
		{
			std::vector<Texture> res;
			for (uint32_t i = 0; i < utils::capabilitiesGetImageCount(Singletons::physical_device, Singletons::surface); ++i)
			{
				res.emplace_back(extent);
			}
			return res;
		}

		[[nodiscard]] std::vector<vk::raii::Framebuffer> createFrameBuffers(
			const LogicalDevice& device, const ViewportRenderPass& render_pass)
		{
			std::vector<vk::raii::Framebuffer> result;

			for (size_t i = 0; i < color_buffers_.size(); i++)
			{
				const vk::ImageView attachments[] = {
					*color_buffers_[i].getImageView(),
					**depth_buffer_
				};

				const vk::FramebufferCreateInfo framebufferInfo
				{
					{}, **render_pass,
					attachments, extent_.width, extent_.height, 1
				};

				result.push_back(device->createFramebuffer(framebufferInfo));
			}

			return result;
		}
	};
}
