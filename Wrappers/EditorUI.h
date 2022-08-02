#pragma once
#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "Singletons.h"
#include "ImGuiRaii.h"
#include "ImGUIRenderPass.h"
#include "ImGUISwapChain.h"
#include "imgui_internal.h"
#include "Viewport.h"

namespace dmbrn
{
	class EditorUI
	{
	public:
		EditorUI(const Singletons& singletons) :
			render_pass_(singletons.surface, singletons.physical_device, singletons.device),
			swap_chain_(singletons, render_pass_),
			im_gui_(singletons, render_pass_),
			viewport_(singletons)
		{
			
		}


		void drawFrame(Singletons& singletons, float delta_time)
		{
			const EditorFrame& frame = swap_chain_.getFrame(currentFrame);

			uint32_t imageIndex = newFrame(singletons.device, frame);

			ImGui::DockSpaceOverViewport();
			//showAppMainMenuBar();
			//ImGui::ShowDemoWindow();

			viewport_.newImGuiFrame(singletons, imageIndex);

			render(singletons, frame, imageIndex);

			submitAndPresent(singletons, frame, imageIndex);

			currentFrame = (currentFrame + 1) % singletons.device.MAX_FRAMES_IN_FLIGHT;
		}

	private:
		ImGUIRenderPass render_pass_;
		ImGUISwapChain swap_chain_;
		ImGuiRaii im_gui_;
		Viewport viewport_;

		uint32_t currentFrame = 0;

		static void showAppMainMenuBar()
		{
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("New"))
					{
					}
					if (ImGui::MenuItem("Open"))
					{
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Edit"))
				{
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}
		}

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
		void render(const Singletons& singletons, const EditorFrame& frame, uint32_t imageIndex)
		{
			ImGuiIO& io = ImGui::GetIO();
			ImGui::Render();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}

			const vk::raii::CommandBuffer& command_buffer = frame.command_buffer;

			command_buffer.begin({vk::CommandBufferUsageFlags()});

			viewport_.render(singletons.device, command_buffer,currentFrame, imageIndex);

			vk::ClearValue clearValue;
			clearValue.color = vk::ClearColorValue(std::array<float, 4>({0.5f, 0.5f, 0.5f, 1.0f}));
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

		void submitAndPresent(Singletons& singletons, const EditorFrame& frame, uint32_t imageIndex)
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
