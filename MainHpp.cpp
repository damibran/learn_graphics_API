#define GLFW_INCLUDE_VULKAN
#include <chrono>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <thread>

#include "Wrappers/Singletons/Singletons.h"
#include "Main/Scene.h"
#include "EditorUI/EditorUI.h"

#include "imgui.h"

namespace dmbrn
{
	class HelloTriangleApplication
	{
	public:
		HelloTriangleApplication():
			scene_({1280, 720}),
			editor_ui_(scene_)
		{
		}

		void run()
		{
			while (!Singletons::window.windowShouldClose())
			{
				tp2_ = std::chrono::system_clock::now();
				const std::chrono::duration<float> elapsed_time = tp2_ - tp1_;
				tp1_ = tp2_;
				const float delta_time = elapsed_time.count();

				glfwPollEvents();
				editor_ui_.drawFrame(delta_time);

				Singletons::window.setWindowTitle("Vulkan. FPS: " + std::to_string(1.0f / delta_time));
			}
			Singletons::device->waitIdle();
		}

	private:
		Scene scene_;
		EditorUI editor_ui_;

		std::chrono::system_clock::time_point tp1_ = std::chrono::system_clock::now();
		std::chrono::system_clock::time_point tp2_ = std::chrono::system_clock::now();
	};
}

int main()
{
	try
	{
		dmbrn::HelloTriangleApplication app{};
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
