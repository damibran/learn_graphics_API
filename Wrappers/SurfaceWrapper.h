#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "InstanceWrapper.h"
#include "GLFWwindowWrapper.h"

namespace dmbrn
{
	class SurfaceWrapper
	{
	public:
		SurfaceWrapper(const InstanceWrapper& instance, const GLFWwindowWrapper& window)
		{
			VkSurfaceKHR raw_surface;
			glfwCreateWindowSurface(**instance, window.data(), nullptr, &raw_surface);
			surface_ = std::make_unique<vk::raii::SurfaceKHR>(*instance, raw_surface, nullptr);
		}

		vk::raii::SurfaceKHR& operator*()const
		{
			return *surface_;
		}

		vk::raii::SurfaceKHR* operator->()const
		{
			return surface_.get();
		}

	private:
		std::unique_ptr<vk::raii::SurfaceKHR> surface_;
	};
}
