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
#include "Wrappers/RenderPass.h"
#include "Wrappers/SwapChain.h"
#include "Wrappers/DescriptorSetLayout.h" // may be it should be a part of descriptor sets
#include "Wrappers/GraphicsPipeline.h"
#include "Wrappers/CommandPool.h"
#include "Wrappers/Model.h"
#include "Wrappers/UniformBuffers.h"
#include "Wrappers/DescriptorSets.h"
#include "Wrappers/CommandBuffers.h"

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
			graphics_pipeline_(device_, render_pass_, descriptor_set_layout_),
			command_pool_(physical_device_, device_),
			model_("Models\\Barrel\\barell.obj", physical_device_, device_, command_pool_, gragraphics_queue_),
			uniform_buffers_(physical_device_, device_),
			descriptor_sets_(device_, descriptor_set_layout_, uniform_buffers_),
			command_buffers_(device_, command_pool_)
		{
			vk::SemaphoreCreateInfo semaphoreInfo{};

			vk::FenceCreateInfo fenceInfo
			{
				vk::FenceCreateFlagBits::eSignaled
			};

			for (size_t i = 0; i < device_.MAX_FRAMES_IN_FLIGHT; i++) {
				image_available_semaphores_.push_back(device_->createSemaphore(semaphoreInfo));
				render_finished_semaphores_.push_back(device_->createSemaphore(semaphoreInfo));
				in_flight_fences_.push_back(device_->createFence(fenceInfo));
			}
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

				std::this_thread::sleep_for(std::chrono::milliseconds(14));
			}
			device_->waitIdle();
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
		RenderPass render_pass_;
		SwapChain swap_chain_;
		DescriptorSetLayout descriptor_set_layout_;
		GraphicsPipeline graphics_pipeline_;
		CommandPool command_pool_;
		Model model_;
		UniformBuffers uniform_buffers_;
		DescriptorSets descriptor_sets_;
		CommandBuffers command_buffers_;
		std::vector<vk::raii::Semaphore> image_available_semaphores_;
		std::vector<vk::raii::Semaphore> render_finished_semaphores_;
		std::vector<vk::raii::Fence> in_flight_fences_;

		std::chrono::system_clock::time_point tp1_ = std::chrono::system_clock::now();
		std::chrono::system_clock::time_point tp2_ = std::chrono::system_clock::now();

		uint32_t currentFrame = 0;

		void drawFrame(float delta_time)
		{
			device_->waitForFences(*in_flight_fences_[currentFrame], true, UINT64_MAX);

			auto result = swap_chain_->acquireNextImage(UINT64_MAX, *image_available_semaphores_[currentFrame]);

			uint32_t imageIndex = result.second;

			updateUniformBuffer(currentFrame, delta_time);

			device_->resetFences(*in_flight_fences_[currentFrame]);

			command_buffers_[currentFrame].reset();

			command_buffers_.recordCommandBuffer(device_, render_pass_, graphics_pipeline_,
				swap_chain_, model_, descriptor_sets_,
				currentFrame, imageIndex);

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

			currentFrame = (currentFrame + 1) % device_.MAX_FRAMES_IN_FLIGHT;
		}

		void updateUniformBuffer(uint32_t currentImage, float delta_t)
		{
			const float speed = 90;

			static float objAngle = 0;

			objAngle += delta_t * glm::radians(speed);

			UniformBuffers::UniformBufferObject ubo{};
			ubo.model = glm::rotate(glm::mat4(1.0f), objAngle, glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.proj = glm::perspective(glm::radians(45.0f), swap_chain_.getExtent().width / (float)swap_chain_.getExtent().height, 0.1f, 10.0f);
			ubo.proj[1][1] *= -1;

			void* data = uniform_buffers_.getUBMemory(currentImage).mapMemory(0, sizeof(ubo));
			memcpy(data, &ubo, sizeof(ubo));
			uniform_buffers_.getUBMemory(currentImage).unmapMemory();
		}
	};
}

//int main()
//{
//	try {
//		dmbrn::HelloTriangleApplication app(800, 600);
//		app.run();
//	}
//	catch (const std::exception& e) {
//		std::cerr << e.what() << std::endl;
//		return EXIT_FAILURE;
//	}
//
//	return EXIT_SUCCESS;
//}