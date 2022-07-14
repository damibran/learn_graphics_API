#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <fstream>
#include <optional>
#include <set>

#include "InstanceWrapper.h"
#include "SurfaceWrapper.h"

class PhysicalDeviceWrapper
{
public:
	PhysicalDeviceWrapper(const InstanceWrapper& instance, const SurfaceWrapper& surface)
	{
		vk::raii::PhysicalDevices physical_devices(*instance);

		bool finded = false;
		for (const auto& device : physical_devices) {
			if (isDeviceSuitable(device, surface)) {
				physical_device_ = std::make_unique<vk::raii::PhysicalDevice>(device);
				finded = true;
				queue_family_indices_ = findQueueFamilies(*physical_device_, surface);
				break;
			}
		}

		if (!finded) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	vk::raii::PhysicalDevice& operator*()const
	{
		return *physical_device_;
	}

	vk::raii::PhysicalDevice* operator->()const
	{
		return physical_device_.get();
	}

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	QueueFamilyIndices getQueueFamilyIndices()const
	{
		return queue_family_indices_;
	}

private:
	std::unique_ptr<vk::raii::PhysicalDevice> physical_device_;
	QueueFamilyIndices queue_family_indices_;

	struct SwapChainSupportDetails {
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	bool isDeviceSuitable(const vk::raii::PhysicalDevice& device, const SurfaceWrapper& surface)
	{
		QueueFamilyIndices indices = findQueueFamilies(device, surface);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		vk::PhysicalDeviceFeatures supportedFeatures = device.getFeatures();

		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	static QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice& device, const SurfaceWrapper& surface)
	{
		QueueFamilyIndices indices;

		std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {

			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
				indices.graphicsFamily = i;
			}

			if (device.getSurfaceSupportKHR(i, **surface)) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	bool checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& device) const
	{
		std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	static SwapChainSupportDetails querySwapChainSupport(const vk::raii::PhysicalDevice& device, const SurfaceWrapper& surface)
	{
		SwapChainSupportDetails details;

		details.capabilities = device.getSurfaceCapabilitiesKHR(**surface);

		details.formats = device.getSurfaceFormatsKHR(**surface);

		details.presentModes = device.getSurfacePresentModesKHR(**surface);

		return details;
	}
};