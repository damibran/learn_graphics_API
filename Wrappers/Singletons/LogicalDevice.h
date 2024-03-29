#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <optional>
#include <set>

#include "PhysicalDevice.h"

namespace dmbrn
{
	/**
	 * \brief wrapper for vkDevice
	 */
	class LogicalDevice
	{
		friend struct Singletons;
	public:
		static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

		const vk::raii::Device& operator*() const
		{
			return device_;
		}

		const vk::raii::Device* operator->() const
		{
			return &device_;
		}

	private:
		vk::raii::Device device_;

		LogicalDevice(const PhysicalDevice& physical_device):
			device_(nullptr)
		{
			const PhysicalDevice::QueueFamilyIndices indices = physical_device.getQueueFamilyIndices();

			std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
			const std::set<uint32_t> uniqueQueueFamilies = {
				indices.graphicsFamily.value(), indices.presentFamily.value()
			};

			float queuePriority = 1.0f;
			for (uint32_t queueFamily : uniqueQueueFamilies)
			{
				const vk::DeviceQueueCreateInfo queueCreateInfo({},
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

			if (enableValidationLayers)
			{
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				createInfo.ppEnabledLayerNames = validationLayers.data();
			}
			else
			{
				createInfo.enabledLayerCount = 0;
			}

			device_ = vk::raii::Device{physical_device->createDevice(createInfo)};
		}
	};
}
