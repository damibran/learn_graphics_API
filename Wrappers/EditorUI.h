#pragma once
#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "CommandPool.h"
#include "ImGUIRenderPass.h"
#include "ImGUISwapChain.h"

namespace dmbrn
{
	class EditorUI
	{
	public:
		EditorUI(const GLFWwindowWrapper& window, const Instance& instance, const Surface& surface,
		         const PhysicalDevice& physical_device, const LogicalDevice& device, const CommandPool& command_pool,
		         const vk::raii::Queue& g_queue) :
			render_pass_(surface, physical_device, device),
			swap_chain_(physical_device, device, surface, window, render_pass_),
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

			imguiPool = device->createDescriptorPool(pool_info);

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

			ImGui_ImplGlfw_InitForVulkan(window.data(), true);

			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = **instance;
			init_info.PhysicalDevice = **physical_device;
			init_info.Device = **device;
			init_info.QueueFamily = physical_device.getQueueFamilyIndices().graphicsFamily.value();
			init_info.Queue = *g_queue;
			init_info.DescriptorPool = *imguiPool;
			init_info.Subpass = 0;
			init_info.MinImageCount = 2;
			init_info.ImageCount = device.MAX_FRAMES_IN_FLIGHT;
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			ImGui_ImplVulkan_Init(&init_info, **render_pass_);

			vk::raii::CommandBuffer cb = command_pool.beginSingleTimeCommands(device);
			ImGui_ImplVulkan_CreateFontsTexture(*cb);

			command_pool.endSingleTimeCommands(g_queue, cb);
		}

		~EditorUI()
		{
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}

		void drawFrame(float delta_time)
		{
			uint32_t imageIndex = newFrame();

			ImGui::ShowDemoWindow();

			render(imageIndex);

			submitAndPresent(imageIndex);

			currentFrame = (currentFrame + 1) % device_.MAX_FRAMES_IN_FLIGHT;
		}

	private:
		ImGUIRenderPass render_pass_;
		ImGUISwapChain swap_chain_;
		vk::raii::DescriptorPool imguiPool;

		uint32_t currentFrame = 0;

		void submitAndPresent(uint32_t imageIndex)
		{
			const vk::Semaphore waitSemaphores[] = {*image_available_semaphores_[currentFrame]};
			const vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
			const vk::Semaphore signalSemaphores[] = {*render_finished_semaphores_[currentFrame]};

			const vk::SubmitInfo submitInfo
			{
				waitSemaphores,
				waitStages,
				*command_buffers_[currentFrame],
				signalSemaphores
			};

			gragraphics_queue_.submit(submitInfo, *in_flight_fences_[currentFrame]);

			try
			{
				const vk::PresentInfoKHR presentInfo
				{
					signalSemaphores,
					**swap_chain_,
					imageIndex
				};
				present_queue_.presentKHR(presentInfo);
			}
			catch (vk::OutOfDateKHRError e)
			{
				window_.framebufferResized = false;
				swap_chain_.recreate(physical_device_, device_, surface_, window_, render_pass_);
			}
		}

		uint32_t newFrame()
		{
			device_->waitForFences(*in_flight_fences_[currentFrame], true, UINT64_MAX);

			auto result = swap_chain_->acquireNextImage(UINT64_MAX, *image_available_semaphores_[currentFrame]);

			device_->resetFences(*in_flight_fences_[currentFrame]);

			command_buffers_[currentFrame].reset();

			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			return result.second;
		}

		/**
		 * \brief record command buffer with ImGUIRenderPass
		 */
		void render(uint32_t imageIndex)
		{
			ImGuiIO& io = ImGui::GetIO();
			ImGui::Render();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}

			const vk::raii::CommandBuffer& command_buffer = command_buffers_[currentFrame];

			vk::ClearValue clearValue;
			clearValue.color = vk::ClearColorValue(std::array<float, 4>({0.5f, 0.5f, 0.5f, 1.0f}));
			command_buffer.begin({vk::CommandBufferUsageFlags()});
			command_buffer.beginRenderPass({
				                               **render_pass_,
				                               *swap_chain_.getFrameBuffers()[imageIndex],
				                               {{0, 0}, swap_chain_.getExtent()},
				                               1, &clearValue
			                               }, vk::SubpassContents::eInline);

			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *command_buffer);

			command_buffer.endRenderPass();
			command_buffer.end();
		}
	};
}
