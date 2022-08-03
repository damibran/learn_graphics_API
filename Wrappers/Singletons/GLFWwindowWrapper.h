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

			window_ = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
			glfwSetWindowUserPointer(window_, this);
			glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
		}

		GLFWwindow* data() const
		{
			return window_;
		}

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
		{
			const auto app = reinterpret_cast<GLFWwindowWrapper*>(glfwGetWindowUserPointer(window));
			app->framebufferResized = true;
		}

		~GLFWwindowWrapper()
		{
			glfwDestroyWindow(window_);
			glfwTerminate();
		}

		std::pair<int, int> getFrameBufferSize() const
		{
			std::pair<int, int> res;
			glfwGetFramebufferSize(window_, &res.first, &res.second);
			return res;
		}

		bool windowShouldClose() const
		{
			return glfwWindowShouldClose(window_);
		}

		void setWindowTitle(const std::string& s)
		{
			glfwSetWindowTitle(window_, s.c_str());
		}

	private:
		GLFWwindow* window_;
	public:
		bool framebufferResized = false;
	};
}
