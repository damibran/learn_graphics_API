#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Wrappers/Singletons/Surface.h"
#include "Wrappers/Singletons/PhysicalDevice.h"
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
		ViewportSwapChain(vk::Extent2D extent, const Singletons& singletons, const ViewportRenderPass& render_pass):
			extent_(extent),
			color_buffers_(createColorBuffers(extent, singletons.physical_device, singletons.surface, singletons.device,
			                                  singletons.command_pool, singletons.gragraphics_queue)),
			depth_buffer_(singletons.surface, singletons.window, singletons.physical_device, singletons.device),
			framebuffers_(createFrameBuffers(singletons.device, render_pass))
		{
		}

		void recreate(const vk::Extent2D extent, const Singletons& singletons, const ViewportRenderPass& render_pass)
		{
			extent_ = extent;

			singletons.device->waitIdle(); // may be not

			color_buffers_ = createColorBuffers(extent_, singletons.physical_device, singletons.surface,
			                                    singletons.device,
			                                    singletons.command_pool, singletons.gragraphics_queue);
			depth_buffer_ = DepthBuffer(singletons.surface, singletons.window, singletons.physical_device,
			                            singletons.device);
			framebuffers_ = createFrameBuffers(singletons.device, render_pass);
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

		[[nodiscard]] std::vector<Texture> createColorBuffers(vk::Extent2D extent,
		                                                      const PhysicalDevice& physical_device,
		                                                      const Surface& surface,
		                                                      const LogicalDevice& device,
		                                                      const CommandPool& command_pool,
		                                                      const vk::raii::Queue& gragraphics_queue)
		{
			std::vector<Texture> res;
			for (int i = 0; i < utils::capabilitiesGetImageCount(physical_device, surface); ++i)
			{
				res.emplace_back(extent, physical_device, device, command_pool, gragraphics_queue);
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
