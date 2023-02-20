#pragma once

#include "CommandPool.h"
#include "GLFWwindowWrapper.h"
#include "Instance.h"
#include "Surface.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "DescriptorPool.h"

namespace dmbrn
{
	struct Singletons
	{
		Singletons() = delete;

		static inline GLFWwindowWrapper window;
		static inline vk::raii::Context context;
		static inline Instance instance{context};
		static inline Surface surface{instance, window};
		static inline PhysicalDevice physical_device{instance, surface};
		static inline LogicalDevice device{physical_device};
		static inline vk::raii::Queue graphics_queue{
			device->getQueue(physical_device.getQueueFamilyIndices().graphicsFamily.value(), 0)
		};
		static inline vk::raii::Queue present_queue{
			device->getQueue(physical_device.getQueueFamilyIndices().presentFamily.value(), 0)
		};
		static inline CommandPool command_pool{physical_device, device};
		static inline DescriptorPool descriptor_pool{device};
	};
}
