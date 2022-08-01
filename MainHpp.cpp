#define GLFW_INCLUDE_VULKAN
#include <chrono>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <optional>
#include <thread>

#include "Wrappers/Singletons.h"
#include "Wrappers/EditorUI.h"
#include "Wrappers/Model.h"

#include "imgui.h"

namespace dmbrn
{
	class HelloTriangleApplication
	{
	public:
		HelloTriangleApplication(uint32_t width, uint32_t height) :
			singletons_(width,height),
			editor_ui_(singletons_)
		{
		}

		void run()
		{
			while (!singletons_.window.windowShouldClose())
			{
				tp2_ = std::chrono::system_clock::now();
				const std::chrono::duration<float> elapsed_time = tp2_ - tp1_;
				tp1_ = tp2_;
				const float delta_time = elapsed_time.count();

				glfwPollEvents();
				editor_ui_.drawFrame(singletons_,delta_time);

				singletons_.window.setWindowTitle("Vulkan. FPS: " + std::to_string(1.0f / delta_time));
			}
			singletons_.device->waitIdle();
		}

	private:
		Singletons singletons_;
		EditorUI editor_ui_;

		std::chrono::system_clock::time_point tp1_ = std::chrono::system_clock::now();
		std::chrono::system_clock::time_point tp2_ = std::chrono::system_clock::now();
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
