#define GLFW_INCLUDE_VULKAN
#include <chrono>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <optional>
#include <thread>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Wrappers/GLFWwindowWrapper.h"
#include "Wrappers/Instance.h"
#include "Wrappers/Surface.h"
#include "Wrappers/PhysicalDevice.h"
#include "Wrappers/LogicalDevice.h"
#include "Wrappers/ImGUIRenderPass.h"
#include "Wrappers/ImGUISwapChain.h"
#include "Wrappers/DescriptorSetLayout.h" // may be it should be a part of descriptor sets
#include "Wrappers/GraphicsPipeline.h"
#include "Wrappers/CommandPool.h"
#include "Wrappers/Model.h"
#include "Wrappers/UniformBuffers.h"
#include "Wrappers/DescriptorSets.h"
#include "Wrappers/CommandBuffers.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace dmbrn
{
	class HelloTriangleApplication
	{
	public:
		HelloTriangleApplication(uint32_t width, uint32_t height) :
			window_(width, height),
			instance_(context_),
			surface_(instance_, window_),
			physical_device_(instance_, surface_),
			device_(physical_device_),
			gragraphics_queue_(device_->getQueue(physical_device_.getQueueFamilyIndices().graphicsFamily.value(), 0)),
			present_queue_(device_->getQueue(physical_device_.getQueueFamilyIndices().presentFamily.value(), 0)),
			render_pass_(surface_, physical_device_, device_),
			swap_chain_(physical_device_, device_, surface_, window_, render_pass_),
			descriptor_set_layout_(device_),
			//graphics_pipeline_(device_, render_pass_, descriptor_set_layout_),
			command_pool_(physical_device_, device_),
			model_("Models\\Barrel\\barell.obj", physical_device_, device_, command_pool_, gragraphics_queue_),
			//uniform_buffers_(physical_device_, device_),
			//descriptor_sets_(device_, descriptor_set_layout_, uniform_buffers_),
			command_buffers_(device_, command_pool_),
			imguiPool(nullptr)
		{
			vk::SemaphoreCreateInfo semaphoreInfo{};

			vk::FenceCreateInfo fenceInfo
			{
				vk::FenceCreateFlagBits::eSignaled
			};

			for (size_t i = 0; i < device_.MAX_FRAMES_IN_FLIGHT; i++)
			{
				image_available_semaphores_.push_back(device_->createSemaphore(semaphoreInfo));
				render_finished_semaphores_.push_back(device_->createSemaphore(semaphoreInfo));
				in_flight_fences_.push_back(device_->createFence(fenceInfo));
			}

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

			imguiPool = device_->createDescriptorPool(pool_info);

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

			ImGui_ImplGlfw_InitForVulkan(window_.data(), true);

			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = **instance_;
			init_info.PhysicalDevice = **physical_device_;
			init_info.Device = **device_;
			init_info.QueueFamily = physical_device_.getQueueFamilyIndices().graphicsFamily.value();
			init_info.Queue = *gragraphics_queue_;
			init_info.DescriptorPool = *imguiPool;
			init_info.Subpass = 0;
			init_info.MinImageCount = 2;
			init_info.ImageCount = device_.MAX_FRAMES_IN_FLIGHT;
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			ImGui_ImplVulkan_Init(&init_info, **render_pass_);

			vk::raii::CommandBuffer cb = command_pool_.beginSingleTimeCommands(device_);
			ImGui_ImplVulkan_CreateFontsTexture(*cb);

			command_pool_.endSingleTimeCommands(gragraphics_queue_, cb);
		}

		void run()
		{
			while (!window_.windowShouldClose())
			{
				tp2_ = std::chrono::system_clock::now();
				const std::chrono::duration<float> elapsed_time = tp2_ - tp1_;
				tp1_ = tp2_;
				const float delta_time = elapsed_time.count();

				glfwPollEvents();
				drawFrame(delta_time);

				window_.setWindowTitle("Vulkan. FPS: " + std::to_string(1.0f / delta_time));
			}
			device_->waitIdle();
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}

	private:
		GLFWwindowWrapper window_;
		vk::raii::Context context_;
		Instance instance_;
		Surface surface_;
		PhysicalDevice physical_device_;
		LogicalDevice device_;
		vk::raii::Queue gragraphics_queue_;
		vk::raii::Queue present_queue_;
		ImGUIRenderPass render_pass_;
		ImGUISwapChain swap_chain_;
		DescriptorSetLayout descriptor_set_layout_;
		//GraphicsPipeline graphics_pipeline_;
		CommandPool command_pool_;
		Model model_;
		//UniformBuffers uniform_buffers_;
		//DescriptorSets descriptor_sets_;
		CommandBuffers command_buffers_;
		std::vector<vk::raii::Semaphore> image_available_semaphores_;
		std::vector<vk::raii::Semaphore> render_finished_semaphores_;
		std::vector<vk::raii::Fence> in_flight_fences_;

		vk::raii::DescriptorPool imguiPool;

		std::chrono::system_clock::time_point tp1_ = std::chrono::system_clock::now();
		std::chrono::system_clock::time_point tp2_ = std::chrono::system_clock::now();

		uint32_t currentFrame = 0;

		void drawFrame(float delta_time)
		{
			uint32_t imageIndex = newFrame();

			ImGui::ShowDemoWindow();

			render(imageIndex);

			submitAndPresent(imageIndex);

			currentFrame = (currentFrame + 1) % device_.MAX_FRAMES_IN_FLIGHT;
		}

		void submitAndPresent(uint32_t imageIndex)
		{
			const vk::Semaphore waitSemaphores[] = { *image_available_semaphores_[currentFrame] };
			const vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
			const vk::Semaphore signalSemaphores[] = { *render_finished_semaphores_[currentFrame] };

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
				return;
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
			clearValue.color = vk::ClearColorValue(std::array<float, 4>({ 0.5f, 0.5f, 0.5f, 1.0f }));
			command_buffer.begin({ vk::CommandBufferUsageFlags() });
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

		//void updateUniformBuffer(uint32_t currentImage, float delta_t)
		//{
		//	const float speed = 90;
		//
		//	static float objAngle = 0;
		//
		//	objAngle += delta_t * glm::radians(speed);
		//
		//	UniformBuffers::UniformBufferObject ubo{};
		//	ubo.model = rotate(glm::mat4(1.0f), objAngle, glm::vec3(0.0f, 0.0f, 1.0f));
		//	ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		//		glm::vec3(0.0f, 0.0f, 1.0f));
		//	ubo.proj = glm::perspective(glm::radians(45.0f),
		//		swap_chain_.getExtent().width / static_cast<float>(swap_chain_.getExtent().
		//			height), 0.1f,
		//		10.0f);
		//	ubo.proj[1][1] *= -1;
		//
		//	void* data = uniform_buffers_.getUBMemory(currentImage).mapMemory(0, sizeof(ubo));
		//	memcpy(data, &ubo, sizeof(ubo));
		//	uniform_buffers_.getUBMemory(currentImage).unmapMemory();
		//}
	};
}

int main()
{
	try
	{
		dmbrn::HelloTriangleApplication app(1200, 800);
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
