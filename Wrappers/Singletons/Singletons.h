#pragma once

#include "CommandPool.h"
#include "GLFWwindowWrapper.h"
#include "Instance.h"
#include "Surface.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "UnLitDescriptorsStatics.h"

namespace dmbrn
{
	struct Singletons
	{
		Singletons() = delete;

		friend class HelloTriangleApplication;
		friend class EditorUI;
		friend class ImGUIRenderPass;
		friend class ImGUISwapChain;
		friend class EditorFrame;
		friend class ImGuiRaii;
		friend class ViewportRenderPass;
		friend class ViewportSwapChain;
		friend class Texture;
		friend class UnlitTextureMaterial;
		friend class Mesh;

	private:
		static inline GLFWwindowWrapper window{1280, 720};
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
		static inline UnLitDescriptorsStatics un_lit_descriptor_statics{device};
	};
}
