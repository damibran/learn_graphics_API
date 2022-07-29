#pragma once
#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "Singletons.h"
#include "CommandPool.h"
#include "ImGUIRenderPass.h"
#include "ImGUISwapChain.h"

namespace dmbrn
{
	class EditorUI
	{
	public:
		EditorUI(const Singletons& singletons) :
			render_pass_(singletons.surface, singletons.physical_device, singletons.device),
			swap_chain_(singletons, render_pass_),
			imguiPool(nullptr)
		{
			vk::DescriptorPoolSize pool_sizes[] =
			{
				{vk::DescriptorType::eSampler, 1000},
				{vk::DescriptorType::eCombinedImageSampler, 1000},
				{vk::DescriptorType::eSampledImage, 1000},
				{vk::DescriptorType::eStorageImage, 1000},
				{vk::DescriptorType::eUniformTexelBuffer, 1000},
				{vk::DescriptorType::eStorageTexelBuffer, 1000},
				{vk::DescriptorType::eUniformBuffer, 1000},
				{vk::DescriptorType::eStorageBuffer, 1000},
				{vk::DescriptorType::eUniformBufferDynamic, 1000},
				{vk::DescriptorType::eStorageBufferDynamic, 1000},
				{vk::DescriptorType::eInputAttachment, 1000}
			};

			vk::DescriptorPoolCreateInfo pool_info
			{
				{vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet},
				1000, pool_sizes
			};

			imguiPool = singletons.device->createDescriptorPool(pool_info);

			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			(void)io;
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

			ImGui::StyleColorsDark();

			// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
			ImGuiStyle& style = ImGui::GetStyle();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				style.WindowRounding = 0.0f;
				style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			}

			ImGui_ImplGlfw_InitForVulkan(singletons.window.data(), true);

			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = **singletons.instance;
			init_info.PhysicalDevice = **singletons.physical_device;
			init_info.Device = **singletons.device;
			init_info.QueueFamily = singletons.physical_device.getQueueFamilyIndices().graphicsFamily.value();
			init_info.Queue = *singletons.gragraphics_queue;
			init_info.DescriptorPool = *imguiPool;
			init_info.Subpass = 0;
			init_info.MinImageCount = 2; // idk what values to put here
			init_info.ImageCount = singletons.device.MAX_FRAMES_IN_FLIGHT; // idk what values to put here
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			ImGui_ImplVulkan_Init(&init_info, **render_pass_);

			vk::raii::CommandBuffer cb = singletons.command_pool.beginSingleTimeCommands(singletons.device);
			ImGui_ImplVulkan_CreateFontsTexture(*cb);

			singletons.command_pool.endSingleTimeCommands(singletons.gragraphics_queue, cb);
		}

		~EditorUI()
		{
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}

		void drawFrame(Singletons& singletons,float delta_time)
		{
			const EditorFrame& frame = swap_chain_.getFrame(currentFrame);

			uint32_t imageIndex = newFrame(singletons.device,frame);

			ImGui::ShowDemoWindow();

			render(frame,imageIndex);

			submitAndPresent(singletons,frame,imageIndex);

			currentFrame = (currentFrame + 1) % singletons.device.MAX_FRAMES_IN_FLIGHT;
		}

	private:
		ImGUIRenderPass render_pass_;
		ImGUISwapChain swap_chain_;
		vk::raii::DescriptorPool imguiPool;

		uint32_t currentFrame = 0;

		uint32_t newFrame(const LogicalDevice& device, const EditorFrame& frame)
		{
			device->waitForFences(*frame.in_flight_fence, true, UINT64_MAX);

			auto result = swap_chain_->acquireNextImage(UINT64_MAX, *frame.image_available_semaphore);

			device->resetFences(*frame.in_flight_fence);

			frame.command_buffer.reset();

			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			return result.second;
		}

		/**
		* \brief record command buffer with ImGUIRenderPass
		*/
		void render(const EditorFrame& frame,uint32_t imageIndex)
		{
			ImGuiIO& io = ImGui::GetIO();
			ImGui::Render();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}

			const vk::raii::CommandBuffer& command_buffer = frame.command_buffer;

			vk::ClearValue clearValue;
			clearValue.color = vk::ClearColorValue(std::array<float, 4>({0.5f, 0.5f, 0.5f, 1.0f}));
			command_buffer.begin({vk::CommandBufferUsageFlags()});
			command_buffer.beginRenderPass({
				                               **render_pass_,
				                               *swap_chain_.getFrame(imageIndex).frame_buffer,
				                               {{0, 0}, swap_chain_.getExtent()},
				                               1, &clearValue
			                               }, vk::SubpassContents::eInline);

			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *command_buffer);

			command_buffer.endRenderPass();
			command_buffer.end();
		}

		void submitAndPresent(Singletons& singletons,const EditorFrame& frame,uint32_t imageIndex)
		{
			const vk::Semaphore waitSemaphores[] = {*frame.image_available_semaphore};
			const vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
			const vk::Semaphore signalSemaphores[] = {*frame.render_finished_semaphore};

			const vk::SubmitInfo submitInfo
			{
				waitSemaphores,
				waitStages,
				*frame.command_buffer,
				signalSemaphores
			};

			singletons.gragraphics_queue.submit(submitInfo, *frame.in_flight_fence);

			try
			{
				const vk::PresentInfoKHR presentInfo
				{
					signalSemaphores,
					**swap_chain_,
					imageIndex
				};
				singletons.present_queue.presentKHR(presentInfo);
			}
			catch (vk::OutOfDateKHRError e)
			{
				singletons.window.framebufferResized = false;
				swap_chain_.recreate(singletons, render_pass_);
			}
		}
	};
}
