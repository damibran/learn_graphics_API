#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <fstream>
#include <optional>
#include <set>

#include "GLFWwindowWrapper.h"
#include "Instance.h"
#include "Surface.h"
#include "PhysicalDevice.h"

namespace dmbrn
{
	class LogicalDevice
	{
	public:
		const int MAX_FRAMES_IN_FLIGHT = 2;

		LogicalDevice(const PhysicalDevice& physical_device, const Surface& surface)
		{
			PhysicalDevice::QueueFamilyIndices indices = physical_device.getQueueFamilyIndices();

			std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

			float queuePriority = 1.0f;
			for (uint32_t queueFamily : uniqueQueueFamilies) {
				vk::DeviceQueueCreateInfo queueCreateInfo({},
					queueFamily,
					1,
					&queuePriority);
				queueCreateInfos.push_back(queueCreateInfo);
			}

			vk::PhysicalDeviceFeatures deviceFeatures{};
			deviceFeatures.samplerAnisotropy = VK_TRUE;

			vk::DeviceCreateInfo createInfo{};

			createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			createInfo.pQueueCreateInfos = queueCreateInfos.data();

			createInfo.pEnabledFeatures = &deviceFeatures;

			createInfo.enabledExtensionCount = static_cast<uint32_t>(physical_device.deviceExtensions.size());
			createInfo.ppEnabledExtensionNames = physical_device.deviceExtensions.data();

			if (enableValidationLayers) {
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				createInfo.ppEnabledLayerNames = validationLayers.data();
			}
			else {
				createInfo.enabledLayerCount = 0;
			}

			device_ = std::make_unique<vk::raii::Device>(physical_device->createDevice(createInfo));
		}

		const vk::raii::Device& operator*()const
		{
			return *device_;
		}

		const vk::raii::Device* operator->()const
		{
			return device_.get();
		}

	private:
		std::unique_ptr<vk::raii::Device> device_;
	};
}