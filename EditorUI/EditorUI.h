#pragma once
#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <ImGuizmo.h>

#include "Wrappers/Singletons/Singletons.h"
#include "Helpers/ImGuiRaii.h"
#include "Helpers/ImGUIRenderPass.h"
#include "Helpers/ImGUISwapChain.h"
#include "Viewport/Viewport.h"
#include "SceneTree.h"
#include "Inspector.h"

namespace dmbrn
{
	class EditorUI
	{
	public:
		EditorUI(Scene& scene) :
			render_pass_(),
			swap_chain_(render_pass_),
			im_gui_(render_pass_),
			scene_(scene),
			scene_tree_(scene_),
			inspector_(scene_tree_),
			viewport_(scene_, scene_tree_.getSelected()),
			viewport2_(scene_, scene_tree_.getSelected()
			           , "Viewport 2")
		{
			Renderer::setRenderPass(*Viewport::render_pass_);
			ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
		}

		void drawFrame(time_point g_time,float delta_time)
		{
			const EditorFrame& frame = swap_chain_.getFrame(current_frame_);

			uint32_t imageIndex = newFrame(Singletons::device, frame);

			beginDockSpace();

			showAppMainMenuBar();
			ImGui::ShowDemoWindow();

			viewport_.newImGuiFrame(delta_time, current_frame_, imageIndex);
			viewport2_.newImGuiFrame(delta_time, current_frame_,imageIndex);
			scene_tree_.newImGuiFrame();
			inspector_.newImGuiFrame(current_frame_);
			drawStatsWindow();

			endDockSpace();

			if(show_model_import)
				showImportWindow();

			scene_.updateAnimations(g_time,current_frame_);
			scene_.updateGlobalTransforms(current_frame_);
			scene_.updatePerStaticModelData(current_frame_);
			scene_.updatePerSkeletalData(current_frame_);

			render(Singletons::device, frame, imageIndex);

			submitAndPresent(Singletons::present_queue, Singletons::graphics_queue, Singletons::window, frame,
			                 imageIndex);

			current_frame_ = (current_frame_ + 1) % Singletons::device.MAX_FRAMES_IN_FLIGHT;
		}

	private:
		ImGUIRenderPass render_pass_;
		ImGUISwapChain swap_chain_;
		ImGuiRaii im_gui_;
		Scene& scene_;
		SceneTree scene_tree_;
		Inspector inspector_;
		Viewport viewport_;
		Viewport viewport2_;
		bool show_model_import = false;
		std::string model_path;

		uint32_t current_frame_ = 0;

		void drawStatsWindow()
		{
			ImGui::Begin("Stats");

			ImGui::Text(("Number of entities created so far: " + std::to_string(scene_.getCountOfEntities())).c_str());

			ImGui::Text(("Count of unique static meshes: " + std::to_string(Mesh::MeshRenderData::getRegistrySize())).c_str());

			ImGui::Text(("Count of unique static meshes: " + std::to_string(SkeletalMesh::SkeletalMeshRenderData::getRegistrySize())).c_str());

			ImGui::Text(("Count of unique materials: " + std::to_string(DiffusionMaterial::getRegistrySize())).c_str());

			ImGui::End();
		}

		void showImportWindow()
		{
			if (!ImGui::Begin("Model import", &show_model_import))
			{
				ImGui::End();
				return;
			}

			char buf[256]={0};

			strcpy_s(buf, sizeof(buf), model_path.c_str());

			if (ImGui::InputText("Model path", buf, sizeof(buf)))
			{
				model_path = std::string(buf);
			}

			if(ImGui::Button("Import"))
			{
				//scene_.addModel(model_path);
				show_model_import = false;
			}

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
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
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
					ImGui::MenuItem("Import model",NULL, &show_model_import);

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
			ImGuizmo::BeginFrame();

			return result.second;
		}

		/**
		* \brief record command buffer with ImGUIRenderPass
		*/
		void render(const LogicalDevice& device, const EditorFrame& frame, uint32_t imageIndex)
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

			viewport_.render(device, command_buffer, current_frame_, imageIndex);
			viewport2_.render(device, command_buffer, current_frame_, imageIndex);

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
