#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Instance.h"
#include "GLFWwindowWrapper.h"

namespace dmbrn
{
	class Surface
	{
	public:
		Surface(const Instance& instance, const GLFWwindowWrapper& window) :
			surface_(nullptr)
		{
			VkSurfaceKHR raw_surface;
			glfwCreateWindowSurface(**instance, window.data(), nullptr, &raw_surface);
			surface_ = vk::raii::SurfaceKHR{*instance, raw_surface, nullptr};
		}

		const vk::raii::SurfaceKHR& operator*()const
		{
			return surface_;
		}

		const vk::raii::SurfaceKHR* operator->()const
		{
			return &surface_;
		}

	private:
		vk::raii::SurfaceKHR surface_;
	};
}
