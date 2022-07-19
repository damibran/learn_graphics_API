#pragma once
#include <GLFW/glfw3.h>

namespace dmbrn
{
	class GLFWwindowWrapper
	{
	public:
		GLFWwindowWrapper(uint32_t width, uint32_t height)
		{
			glfwInit();

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

			window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
			glfwSetWindowUserPointer(window, this);
			glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
		}

		GLFWwindow* data() const
		{
			return window;
		}

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
			auto app = reinterpret_cast<GLFWwindowWrapper*>(glfwGetWindowUserPointer(window));
			app->framebufferResized = true;
		}

		~GLFWwindowWrapper()
		{
			glfwDestroyWindow(window);
			glfwTerminate();
		}

		std::pair<int, int> getFrameBufferSize()const
		{
			std::pair<int, int> res;
			glfwGetFramebufferSize(window, &res.first, &res.second);
			return res;
		}

		bool windowShouldClose()const
		{
			return glfwWindowShouldClose(window);
		}

	private:
		GLFWwindow* window;
	public:
		bool framebufferResized = false;
	};
}