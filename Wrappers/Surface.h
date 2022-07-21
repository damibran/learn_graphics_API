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
			surface_(createSurface(instance, window))
		{
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

		[[nodiscard]] static vk::raii::SurfaceKHR createSurface(const Instance& instance, const GLFWwindowWrapper& window)
		{
			VkSurfaceKHR raw_surface;
			glfwCreateWindowSurface(**instance, window.data(), nullptr, &raw_surface);
			return  {*instance, raw_surface, nullptr};
		}
	};
}
