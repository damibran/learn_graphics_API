#define GLFW_INCLUDE_VULKAN
#include <chrono>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <thread>

using duration = std::chrono::duration<double>;
using sys_clock = std::chrono::system_clock;
using time_point = std::chrono::time_point<sys_clock, duration>;

#include "Wrappers/Singletons/Singletons.h"
#include "ECS/Scene.h"
#include "EditorUI/EditorUI.h"

namespace dmbrn
{
	class HelloTriangleApplication
	{
	public:
		HelloTriangleApplication():
			scene_(),
			editor_ui_(scene_)
		{
		}

		void run()
		{
			while (!Singletons::window.windowShouldClose())
			{
				tp2_ = sys_clock::now();
				const duration elapsed_time = tp2_ - tp1_;
				tp1_ = tp2_;
				const double delta_time = elapsed_time.count();

				glfwPollEvents();
				editor_ui_.drawFrame(delta_time);

				Singletons::window.setWindowTitle("Vulkan. FPS: " + std::to_string(1.0f / delta_time));
			}
			Singletons::device->waitIdle();
		}

	private:
		Scene scene_;
		EditorUI editor_ui_;

		time_point tp1_ = std::chrono::time_point_cast<duration>(sys_clock::now());
		time_point tp2_ = std::chrono::time_point_cast<duration>(sys_clock::now());
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
