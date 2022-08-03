#pragma once

#include "ImGUIRenderPass.h"

namespace dmbrn
{
	class ImGuiRaii
	{
	public:
		ImGuiRaii(const Singletons& singletons, const ImGUIRenderPass& render_pass):
			imguiPool(nullptr)
		{
			vk::DescriptorPoolSize pool_sizes[] =
			{
				{vk::DescriptorType::eSampler, 1000},
				{vk::DescriptorType::eCombinedImageSampler, 1000},
				{vk::DescriptorType::eSampledImage, 1000},
				{vk::DescriptorType::eStorageImage, 1000},
				{vk::DescriptorType::eUniformTexelBuffer, 1000},
				{vk::DescriptorType::eStorageTexelBuffer, 1000},
				{vk::DescriptorType::eUniformBuffer, 1000},
				{vk::DescriptorType::eStorageBuffer, 1000},
				{vk::DescriptorType::eUniformBufferDynamic, 1000},
				{vk::DescriptorType::eStorageBufferDynamic, 1000},
				{vk::DescriptorType::eInputAttachment, 1000}
			};

			vk::DescriptorPoolCreateInfo pool_info
			{
				{vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet},
				1000, pool_sizes
			};

			imguiPool = singletons.device->createDescriptorPool(pool_info);

			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			(void)io;
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
			ImGui::StyleColorsDark();

			// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
			ImGuiStyle& style = ImGui::GetStyle();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				style.WindowRounding = 0.0f;
				style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			}

			ImGui_ImplGlfw_InitForVulkan(singletons.window.data(), true);

			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = **singletons.instance;
			init_info.PhysicalDevice = **singletons.physical_device;
			init_info.Device = **singletons.device;
			init_info.QueueFamily = singletons.physical_device.getQueueFamilyIndices().graphicsFamily.value();
			init_info.Queue = *singletons.gragraphics_queue;
			init_info.DescriptorPool = *imguiPool;
			init_info.Subpass = 0;
			init_info.MinImageCount = 2; // idk what values to put here
			init_info.ImageCount = singletons.device.MAX_FRAMES_IN_FLIGHT; // idk what values to put here
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			ImGui_ImplVulkan_Init(&init_info, **render_pass);

			vk::raii::CommandBuffer cb = singletons.command_pool.beginSingleTimeCommands(singletons.device);

			ImGui_ImplVulkan_CreateFontsTexture(*cb);

			singletons.command_pool.endSingleTimeCommands(singletons.gragraphics_queue, cb);
		}
				
		~ImGuiRaii()
		{
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}

	private:
		vk::raii::DescriptorPool imguiPool;
	};
}