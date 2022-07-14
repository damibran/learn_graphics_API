#pragma once
#include <GLFW/glfw3.h>

class GLFWwindowWraper
{
public:
	GLFWwindowWraper(uint32_t width, uint32_t height)
	{
		glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<GLFWwindowWraper*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

	~GLFWwindowWraper()
	{
		glfwDestroyWindow(window);
        glfwTerminate();
	}

private:
	GLFWwindow* window;
public:
	bool framebufferResized = false;
};