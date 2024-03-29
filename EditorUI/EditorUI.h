#pragma once
#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <ImGuizmo.h>

#include "Wrappers/Singletons/Singletons.h"
#include "Helpers/ImGuiRaii.h"
#include "Helpers/EditorRenderPass.h"
#include "Helpers/EditorSwapChain.h"
#include "Viewport/Viewport.h"
#include "SceneTree.h"
#include "Inspector.h"
#include "Sequencer.h"

#include <ImCurveEdit.h>

namespace dmbrn
{
	/**
	 * \brief is main orchestrator and container for all UI elements
	 */
	class EditorUI
	{
	public:
		EditorUI(Scene& scene) :
			render_pass_(),
			swap_chain_(render_pass_),
			im_gui_(render_pass_),
			scene_(scene),
			scene_tree_(scene_),
			sequencer_(scene.getAnimationSequence()),
			inspector_(scene_, scene_tree_, sequencer_),
			viewport_(scene_, sequencer_, scene_tree_.getSelected()),
			viewport2_(scene_, sequencer_, scene_tree_.getSelected()
			           , "Viewport 2")
		{
			Renderer::setRenderPass(*Viewport::render_pass_);
			ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
		}

		/**
		 * \brief draw UI, update scene both CPU and GPU states, record and submit command buffers
		 * \param delta_time time of previous frame in ms
		 */
		void drawFrame(double delta_time)
		{
			// get current Editor Frame from swap chain
			const EditorFrame& frame = swap_chain_.getFrame(current_frame_);

			// get an image index of it and be sure that all synchronization is done
			const uint32_t imageIndex = newFrame(Singletons::device, frame);

			// begin drawing all the UI
			beginDockSpace();

			showAppMainMenuBar();
			ImGui::ShowDemoWindow();

			// draw all windows
			viewport_.newImGuiFrame(delta_time, current_frame_, imageIndex);
			viewport2_.newImGuiFrame(delta_time, current_frame_, imageIndex);
			scene_tree_.newImGuiFrame();
			inspector_.newImGuiFrame(current_frame_);
			drawStatsWindow();
			drawSequencer(static_cast<float>(delta_time));

			// end drawing all the UI
			endDockSpace();

			// update transforms according to current animation states
			scene_.updateAnimations(sequencer_.getCurrentFrame(), current_frame_);
			// hierarchically update transforms
			scene_.updateGlobalTransforms(current_frame_);
			// update GPU data of static models
			scene_.updatePerStaticModelData(current_frame_);
			// update GPU data of skeletal modes
			scene_.updatePerSkeletalData(current_frame_);

			// record render commands to frame command buffer
			render(frame, imageIndex);

			// submit command buffer to queue and call present
			submitAndPresent(Singletons::present_queue, Singletons::graphics_queue, Singletons::window, frame,
			                 imageIndex);

			current_frame_ = (current_frame_ + 1) % Singletons::device.MAX_FRAMES_IN_FLIGHT;
		}

	private:
		EditorRenderPass render_pass_;
		EditorSwapChain swap_chain_;
		ImGuiRaii im_gui_;
		Scene& scene_;
		SceneTree scene_tree_;
		Sequencer sequencer_;
		Inspector inspector_;
		Viewport viewport_;
		Viewport viewport2_;

		uint32_t current_frame_ = 0;

		void drawStatsWindow()
		{
			ImGui::Begin("Stats");

			ImGui::Text(("Number of entities created so far: " + std::to_string(scene_.getCountOfEntities())).c_str());

			ImGui::Text(
				("Count of unique static meshes: " + std::to_string(Mesh::MeshRenderData::getRegistrySize())).c_str());

			ImGui::Text(
				("Count of unique skeletal meshes: " + std::to_string(
					SkeletalMesh::SkeletalMeshRenderData::getRegistrySize())).c_str());

			ImGui::Text(("Count of unique materials: " + std::to_string(DiffusionMaterial::getRegistrySize())).c_str());

			ImGui::End();
		}

		void beginDockSpace()
		{
			bool p_open = true;
			static bool opt_fullscreen = true;
			static bool opt_padding = false;
			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

			// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
			// because it would be confusing to have two docking targets within each others.ImGuiWindowFlags_MenuBar
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
			if (opt_fullscreen)
			{
				const ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImGui::SetNextWindowPos(viewport->WorkPos);
				ImGui::SetNextWindowSize(viewport->WorkSize);
				ImGui::SetNextWindowViewport(viewport->ID);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
				window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
			}
			else
			{
				dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
			}

			// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
			// and handle the pass-thru hole, so we ask Begin() to not render a background.
			if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
				window_flags |= ImGuiWindowFlags_NoBackground;

			// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
			// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
			// all active windows docked into it will lose their parent and become undocked.
			// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
			// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
			if (!opt_padding)
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("DockSpace Demo", &p_open, window_flags);
			if (!opt_padding)
				ImGui::PopStyleVar();

			if (opt_fullscreen)
				ImGui::PopStyleVar(2);

			// Submit the DockSpace
			const ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				const ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}
			else
			{
				throw std::exception("Turn on docking");
			}
		}

		void endDockSpace()
		{
			ImGui::End();
			const ImGuiIO& io = ImGui::GetIO();
			ImGui::Render();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}
		}

		void showAppMainMenuBar()
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
				if (ImGui::BeginMenu("Scene"))
				{
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}
		}

		void drawSequencer(float d_time)
		{
			if (ImGui::Begin("Sequencer"))
			{
				sequencer_.draw(
					d_time, Sequencer::SEQUENCER_EDIT_STARTEND | Sequencer::SEQUENCER_ADD | Sequencer::SEQUENCER_DEL
					| Sequencer::SEQUENCER_COPYPASTE | Sequencer::SEQUENCER_CHANGE_FRAME);
			}
			ImGui::End();
		}

		/**
		 * \brief get an image index for frame and be sure that all synchronization is done
		 * \param device vulkan logical device for fence manipulation
		 * \param frame editor ui frame data
		 * \return index of acquired image
		 */
		uint32_t newFrame(const LogicalDevice& device, const EditorFrame& frame)
		{
			// wait while all previous work for this frame wasn't done
			device->waitForFences(*frame.in_flight_fence, true, UINT64_MAX);

			// acquire image and signal semaphore when we can start render to it
			const auto result = swap_chain_->acquireNextImage(UINT64_MAX, *frame.image_available_semaphore);

			device->resetFences(*frame.in_flight_fence);

			frame.command_buffer.reset();

			// ImGui new frame
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();

			return result.second;
		}

		/**
		 * \brief record render commands to frame command buffer
		 * \param frame editor frame for command buffer access
		 * \param imageIndex swap chain image index of frame buffer
		 */
		void render(const EditorFrame& frame, uint32_t imageIndex)
		{
			const vk::raii::CommandBuffer& command_buffer = frame.command_buffer;

			command_buffer.begin({vk::CommandBufferUsageFlags()});

			// record commands of viewports
			viewport_.render(command_buffer, current_frame_, imageIndex);
			viewport2_.render(command_buffer, current_frame_, imageIndex);

			// begin imgui render pass
			vk::ClearValue clearValue;
			clearValue.color = vk::ClearColorValue(std::array<float, 4>({0.5f, 0.5f, 0.5f, 1.0f}));
			command_buffer.beginRenderPass({
				                               **render_pass_,
				                               *swap_chain_.getFrame(imageIndex).frame_buffer,
				                               {{0, 0}, swap_chain_.getExtent()},
				                               1, &clearValue
			                               }, vk::SubpassContents::eInline);
			// record imgui commands
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *command_buffer);

			command_buffer.endRenderPass();
			command_buffer.end();
		}

		/**
		 * \brief submit command buffer to queue and call present
		 * \param present present queue
		 * \param graphics graphics queue
		 * \param window window wrapper to handle window resize
		 * \param frame editor frame data
		 * \param imageIndex swap chain image index for presenting
		 */
		void submitAndPresent(vk::raii::Queue& present, vk::raii::Queue& graphics, GLFWwindowWrapper& window,
		                      const EditorFrame& frame, uint32_t imageIndex)
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

			graphics.submit(submitInfo, *frame.in_flight_fence);

			try
			{
				const vk::PresentInfoKHR presentInfo
				{
					signalSemaphores,
					**swap_chain_,
					imageIndex
				};
				present.presentKHR(presentInfo);
			}
			catch (vk::OutOfDateKHRError e)
			{
				window.framebufferResized = false;
				swap_chain_.recreate(render_pass_);
			}
		}
	};
}
