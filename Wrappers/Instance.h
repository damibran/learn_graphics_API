#pragma once
#include <iostream>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace dmbrn
{
#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	const std::vector<const char*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	class Instance
	{
	public:
		Instance(const vk::raii::Context& context):
			instance_(nullptr),
			debug_messenger_(nullptr)
		{
			const vk::ApplicationInfo application_info
			{
				"First Vulkan",
					1,
					"Engine",
					1,
					VK_API_VERSION_1_3
			};

			vk::InstanceCreateInfo create_info;
			create_info.pApplicationInfo = &application_info;

			auto extensions = getRequiredExtensions();
			create_info.setPEnabledExtensionNames(extensions);

			vk::DebugUtilsMessengerCreateInfoEXT debug_create_info;

			if (enableValidationLayers)
			{
				create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				create_info.ppEnabledLayerNames = validationLayers.data();

				populateDebugMessengerCreateInfo(debug_create_info);
				create_info.pNext = &debug_create_info;
			}
			else
			{
				create_info.enabledLayerCount = 0;
				create_info.pNext = nullptr;
			}

			instance_ = vk::raii::Instance{context,
				create_info};

			if constexpr (!enableValidationLayers)
				return;

			debug_messenger_ = vk::raii::DebugUtilsMessengerEXT{instance_.createDebugUtilsMessengerEXT(debug_create_info)};
		}

		const vk::raii::Instance& operator*()const
		{
			return instance_;
		}

		const vk::raii::Instance* operator->()const
		{
			return &instance_;
		}

	private:
		vk::raii::Instance instance_;
		vk::raii::DebugUtilsMessengerEXT debug_messenger_;

		std::vector<const char*> getRequiredExtensions()
		{
			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

			if (enableValidationLayers)
			{
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}

			return extensions;
		}

		static void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
		{
			createInfo.messageSeverity = //vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

			createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
				vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
				vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

			createInfo.pfnUserCallback = debugCallback;
		}

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

			return VK_FALSE;
		}
	};
}