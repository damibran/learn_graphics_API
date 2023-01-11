#pragma once

#include <Wrappers/Singletons/LogicalDevice.h>

namespace dmbrn
{
	class DescriptorPool
	{
	public:
		DescriptorPool(const LogicalDevice& device): pool_(createDescriptorPool(device))
		{
		}

		const vk::raii::DescriptorPool& operator*() const
		{
			return pool_;
		}

	private:
		static const int MAX_SETS = 10;
		static const int MAX_UB = 10;
		static const int MAX_IMG_SAMPLR = 10;

		static vk::raii::DescriptorPool createDescriptorPool(const LogicalDevice& device)
		{
			std::array<vk::DescriptorPoolSize, 2> poolSizes{};

			poolSizes[0] = vk::DescriptorPoolSize
			{
				vk::DescriptorType::eUniformBuffer,
				MAX_UB * static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT)
			};
			poolSizes[1] = vk::DescriptorPoolSize
			{
				vk::DescriptorType::eCombinedImageSampler,
				MAX_IMG_SAMPLR * static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT)
			};

			vk::DescriptorPoolCreateInfo poolInfo
			{
				vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				MAX_SETS * static_cast<uint32_t>(device.MAX_FRAMES_IN_FLIGHT),
				static_cast<uint32_t>(poolSizes.size()),
				poolSizes.data()
			};

			return vk::raii::DescriptorPool{device->createDescriptorPool(poolInfo)};
		}

		vk::raii::DescriptorPool pool_;
	};
}
