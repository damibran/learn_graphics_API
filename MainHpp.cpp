#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <fstream>
#include <optional>
#include <set>

#include "Wrappers/GLFWwindowWrapper.h"
#include "Wrappers/Instance.h"
#include "Wrappers/Surface.h"
#include "Wrappers/PhysicalDevice.h"
#include "Wrappers/LogicalDevice.h"
#include "Wrappers/SwapChain.h"

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
			device_(physical_device_, surface_),
			gragraphics_queue_(device_->getQueue(physical_device_.getQueueFamilyIndices().graphicsFamily.value(), 0)),
			present_queue_(device_->getQueue(physical_device_.getQueueFamilyIndices().presentFamily.value(), 0)),
			swap_chain_(physical_device_, device_, surface_, window_)
		{
			//createRenderPass();
			//createDescriptorSetLayout();
			//createGraphicsPipeline();
			//createFramebuffers();
			//createCommandPool();
			//createTextureImage();
			//createTextureImageView();
			//createTextureSampler();
			//createVertexBuffer();
			//createIndexBuffer();
			//createUniformBuffers();
			//createDescriptorPool();
			//createDescriptorSets();
			//createCommandBuffers();
			//createSyncObjects();
		}

		void run() {

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
		SwapChain swap_chain_;
	};
}

int main()
{
	try {
		dmbrn::HelloTriangleApplication app(800, 600);
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}