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
#include "Helpers/ImGUIRenderPass.h"
#include "Helpers/ImGUISwapChain.h"
#include "Viewport/Viewport.h"
#include "SceneTree.h"
#include "Inspector.h"
#include "Sequencer/ImSequencer.h"

#include <ImCurveEdit.h>
#include <ImSequencer.h>

namespace dmbrn
{
	static const char* SequencerItemTypeNames[] = {"Camera", "Music", "ScreenEffect", "FadeIn", "Animation"};

	struct RampEdit : public ImCurveEdit::Delegate
	{
		RampEdit()
		{
			mPts[0][0] = ImVec2(-10.f, 0);
			mPts[0][1] = ImVec2(20.f, 0.6f);
			mPts[0][2] = ImVec2(25.f, 0.2f);
			mPts[0][3] = ImVec2(70.f, 0.4f);
			mPts[0][4] = ImVec2(120.f, 1.f);
			mPointCount[0] = 5;

			mPts[1][0] = ImVec2(-50.f, 0.2f);
			mPts[1][1] = ImVec2(33.f, 0.7f);
			mPts[1][2] = ImVec2(80.f, 0.2f);
			mPts[1][3] = ImVec2(82.f, 0.8f);
			mPointCount[1] = 4;


			mPts[2][0] = ImVec2(40.f, 0);
			mPts[2][1] = ImVec2(60.f, 0.1f);
			mPts[2][2] = ImVec2(90.f, 0.82f);
			mPts[2][3] = ImVec2(150.f, 0.24f);
			mPts[2][4] = ImVec2(200.f, 0.34f);
			mPts[2][5] = ImVec2(250.f, 0.12f);
			mPointCount[2] = 6;
			mbVisible[0] = mbVisible[1] = mbVisible[2] = true;
			mMax = ImVec2(1.f, 1.f);
			mMin = ImVec2(0.f, 0.f);
		}

		size_t GetCurveCount()
		{
			return 3;
		}

		bool IsVisible(size_t curveIndex)
		{
			return mbVisible[curveIndex];
		}

		size_t GetPointCount(size_t curveIndex)
		{
			return mPointCount[curveIndex];
		}

		uint32_t GetCurveColor(size_t curveIndex)
		{
			uint32_t cols[] = {0xFF0000FF, 0xFF00FF00, 0xFFFF0000};
			return cols[curveIndex];
		}

		ImVec2* GetPoints(size_t curveIndex)
		{
			return mPts[curveIndex];
		}

		virtual ImCurveEdit::CurveType GetCurveType(size_t curveIndex) const { return ImCurveEdit::CurveSmooth; }

		virtual int EditPoint(size_t curveIndex, int pointIndex, ImVec2 value)
		{
			mPts[curveIndex][pointIndex] = ImVec2(value.x, value.y);
			SortValues(curveIndex);
			for (size_t i = 0; i < GetPointCount(curveIndex); i++)
			{
				if (mPts[curveIndex][i].x == value.x)
					return (int)i;
			}
			return pointIndex;
		}

		virtual void AddPoint(size_t curveIndex, ImVec2 value)
		{
			if (mPointCount[curveIndex] >= 8)
				return;
			mPts[curveIndex][mPointCount[curveIndex]++] = value;
			SortValues(curveIndex);
		}

		virtual ImVec2& GetMax() { return mMax; }
		virtual ImVec2& GetMin() { return mMin; }
		virtual unsigned int GetBackgroundColor() { return 0; }
		ImVec2 mPts[3][8];
		size_t mPointCount[3];
		bool mbVisible[3];
		ImVec2 mMin;
		ImVec2 mMax;
	private:
		void SortValues(size_t curveIndex)
		{
			auto b = std::begin(mPts[curveIndex]);
			auto e = std::begin(mPts[curveIndex]) + GetPointCount(curveIndex);
			std::sort(b, e, [](ImVec2 a, ImVec2 b) { return a.x < b.x; });
		}
	};

	struct MySequence : public ImSequencer::SequenceInterface
	{
		// interface with sequencer

		virtual int GetFrameMin() const
		{
			return mFrameMin;
		}

		virtual int GetFrameMax() const
		{
			return mFrameMax;
		}

		virtual int GetItemCount() const { return (int)myItems.size(); }

		virtual int GetItemTypeCount() const { return sizeof(SequencerItemTypeNames) / sizeof(char*); }
		virtual const char* GetItemTypeName(int typeIndex) const { return SequencerItemTypeNames[typeIndex]; }

		virtual const char* GetItemLabel(int index) const
		{
			static char tmps[512];
			snprintf(tmps, 512, "[%02d] %s", index, SequencerItemTypeNames[myItems[index].mType]);
			return tmps;
		}

		virtual void Get(int index, int** start, int** end, int* type, unsigned int* color)
		{
			MySequenceItem& item = myItems[index];
			if (color)
				*color = 0xFFAA8080; // same color for everyone, return color based on type
			if (start)
				*start = &item.mFrameStart;
			if (end)
				*end = &item.mFrameEnd;
			if (type)
				*type = item.mType;
		}

		virtual void Add(int type) { myItems.push_back(MySequenceItem{type, 0, 10, false}); };
		virtual void Del(int index) { myItems.erase(myItems.begin() + index); }
		virtual void Duplicate(int index) { myItems.push_back(myItems[index]); }

		virtual size_t GetCustomHeight(int index) { return myItems[index].mExpanded ? 300 : 0; }

		// my datas
		MySequence() : mFrameMin(0), mFrameMax(0)
		{
		}

		int mFrameMin, mFrameMax;

		struct MySequenceItem
		{
			int mType;
			int mFrameStart, mFrameEnd;
			bool mExpanded;
		};

		std::vector<MySequenceItem> myItems;
		RampEdit rampEdit;

		virtual void DoubleClick(int index)
		{
			if (myItems[index].mExpanded)
			{
				myItems[index].mExpanded = false;
				return;
			}
			for (auto& item : myItems)
				item.mExpanded = false;
			myItems[index].mExpanded = !myItems[index].mExpanded;
		}

		virtual void CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& legendRect,
		                        const ImRect& clippingRect, const ImRect& legendClippingRect)
		{
			static const char* labels[] = {"Translation", "Rotation", "Scale"};

			rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
			rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
			draw_list->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);
			for (int i = 0; i < 3; i++)
			{
				ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + i * 14.f);
				ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (i + 1) * 14.f);
				draw_list->AddText(pta, rampEdit.mbVisible[i] ? 0xFFFFFFFF : 0x80FFFFFF, labels[i]);
				if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
					rampEdit.mbVisible[i] = !rampEdit.mbVisible[i];
			}
			draw_list->PopClipRect();

			ImGui::SetCursorScreenPos(rc.Min);
			ImCurveEdit::Edit(rampEdit, rc.Max - rc.Min, 137 + index, &clippingRect);
		}

		virtual void CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect)
		{
			rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
			rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
			draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < rampEdit.mPointCount[i]; j++)
				{
					float p = rampEdit.mPts[i][j].x;
					if (p < myItems[index].mFrameStart || p > myItems[index].mFrameEnd)
						continue;
					float r = (p - mFrameMin) / float(mFrameMax - mFrameMin);
					float x = ImLerp(rc.Min.x, rc.Max.x, r);
					draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
				}
			}
			draw_list->PopClipRect();
		}
	};

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
			inspector_(scene_,scene_tree_),
			viewport_(scene_, sequencer_,scene_tree_.getSelected()),
			viewport2_(scene_, sequencer_,scene_tree_.getSelected()
			           , "Viewport 2")
		{
			Renderer::setRenderPass(*Viewport::render_pass_);
			ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;

			ex_seq.mFrameMin = 0;
			ex_seq.mFrameMax = 800;
			ex_seq.myItems.push_back(MySequence::MySequenceItem{0, 10, 30, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{1, 20, 30, true});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{3, 12, 60, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{2, 61, 90, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{4, 90, 99, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{0, 10, 30, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{1, 20, 30, true});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{3, 12, 60, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{2, 61, 90, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{4, 90, 99, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{0, 10, 30, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{1, 20, 30, true});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{3, 12, 60, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{2, 61, 90, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{4, 90, 99, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{0, 10, 30, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{1, 20, 30, true});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{3, 12, 60, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{2, 61, 90, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{4, 90, 99, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{0, 10, 30, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{1, 20, 30, true});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{3, 12, 60, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{2, 61, 90, false});
			ex_seq.myItems.push_back(MySequence::MySequenceItem{4, 90, 99, false});
		}

		void drawFrame(time_point g_time, double delta_time)
		{
			const EditorFrame& frame = swap_chain_.getFrame(current_frame_);

			const uint32_t imageIndex = newFrame(Singletons::device, frame);

			beginDockSpace();

			showAppMainMenuBar();
			ImGui::ShowDemoWindow();

			viewport_.newImGuiFrame(delta_time, current_frame_, imageIndex);
			viewport2_.newImGuiFrame(delta_time, current_frame_, imageIndex);
			scene_tree_.newImGuiFrame();
			inspector_.newImGuiFrame(current_frame_);
			drawStatsWindow();
			drawSequencer(static_cast<float>(delta_time));

			// example sequencer
			/*{
				if (ImGui::Begin("Sequencer Example"))
				{
					// let's create the sequencer
					static int selectedEntry = -1;
					static int firstFrame = 0;
					static bool expanded = true;
					static int currentFrame = 100;

					ImGui::PushItemWidth(130);
					ImGui::InputInt("Frame Min", &ex_seq.mFrameMin);
					ImGui::SameLine();
					ImGui::InputInt("Frame ", &currentFrame);
					ImGui::SameLine();
					ImGui::InputInt("Frame Max", &ex_seq.mFrameMax);
					ImGui::PopItemWidth();
					ImSequencer::Sequencer(&ex_seq, &currentFrame, &expanded, &selectedEntry, &firstFrame,
					          ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD |
					          ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE |
					          ImSequencer::SEQUENCER_CHANGE_FRAME);
					// add a UI to edit that particular item
					if (selectedEntry != -1)
					{
						const MySequence::MySequenceItem& item = ex_seq.myItems[selectedEntry];
						ImGui::Text("I am a %s, please edit me", SequencerItemTypeNames[item.mType]);
						// switch (type) ....
					}
				}
				ImGui::End();
			}*/

			endDockSpace();

			if (show_model_import)
				showImportWindow();

			scene_.updateAnimations(sequencer_.getCurrentFrame(), current_frame_);
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
		Sequencer sequencer_;
		Inspector inspector_;
		Viewport viewport_;
		Viewport viewport2_;
		MySequence ex_seq;
		bool show_model_import = false;
		std::string model_path;

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

		void showImportWindow()
		{
			if (!ImGui::Begin("Model import", &show_model_import))
			{
				ImGui::End();
				return;
			}

			char buf[256] = {0};

			strcpy_s(buf, sizeof(buf), model_path.c_str());

			if (ImGui::InputText("Model path", buf, sizeof(buf)))
			{
				model_path = std::string(buf);
			}

			if (ImGui::Button("Import"))
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

		void drawSequencer(float d_time)
		{
			if (ImGui::Begin("Sequencer"))
			{
				sequencer_.draw(d_time,Sequencer::SEQUENCER_EDIT_STARTEND | Sequencer::SEQUENCER_ADD | Sequencer::SEQUENCER_DEL
					| Sequencer::SEQUENCER_COPYPASTE | Sequencer::SEQUENCER_CHANGE_FRAME);

			}
			ImGui::End();
		}

		uint32_t newFrame(const LogicalDevice& device, const EditorFrame& frame)
		{
			device->waitForFences(*frame.in_flight_fence, true, UINT64_MAX);

			const auto result = swap_chain_->acquireNextImage(UINT64_MAX, *frame.image_available_semaphore);

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
			const ImGuiIO& io = ImGui::GetIO();
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
