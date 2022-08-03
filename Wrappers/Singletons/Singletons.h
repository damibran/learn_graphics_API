#pragma once

#include "CommandPool.h"
#include "GLFWwindowWrapper.h"
#include "Instance.h"
#include "Surface.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"

namespace dmbrn
{
	struct Singletons
	{
		Singletons(uint32_t width, uint32_t height):
			window(width, height),
			instance(context),
			surface(instance, window),
			physical_device(instance, surface),
			device(physical_device),
			gragraphics_queue(device->getQueue(physical_device.getQueueFamilyIndices().graphicsFamily.value(), 0)),
			present_queue(device->getQueue(physical_device.getQueueFamilyIndices().presentFamily.value(), 0)),
			command_pool(physical_device, device)
		{
		}

		GLFWwindowWrapper window;
		const vk::raii::Context context;
		const Instance instance;
		const Surface surface;
		const PhysicalDevice physical_device;
		const LogicalDevice device;
		const vk::raii::Queue gragraphics_queue;
		const vk::raii::Queue present_queue;
		const CommandPool command_pool;
	};
}
