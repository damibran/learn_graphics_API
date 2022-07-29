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


#include "Wrappers/Singletons.h"
#include "Wrappers/EditorUI.h"
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
			singletons_(width,height),
			editor_ui_(singletons_),
			command_buffers_(device_, command_pool_)
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
				editor_ui_.drawFrame(delta_time);

				window_.setWindowTitle("Vulkan. FPS: " + std::to_string(1.0f / delta_time));
			}
			device_->waitIdle();
		}

	private:
		Singletons singletons_;
		EditorUI editor_ui_;
		//GraphicsPipeline graphics_pipeline_;
		//UniformBuffers uniform_buffers_;
		//DescriptorSets descriptor_sets_;
		CommandBuffers command_buffers_;
		std::vector<vk::raii::Semaphore> image_available_semaphores_;
		std::vector<vk::raii::Semaphore> render_finished_semaphores_;
		std::vector<vk::raii::Fence> in_flight_fences_;


		std::chrono::system_clock::time_point tp1_ = std::chrono::system_clock::now();
		std::chrono::system_clock::time_point tp2_ = std::chrono::system_clock::now();

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
