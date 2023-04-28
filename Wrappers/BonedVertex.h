#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

namespace dmbrn
{
	struct SkeletalOffsets
	{
		uint32_t renderable_offset;
		uint32_t skeletal_offset;
	};

	struct BonedVertex
	{
		static inline constexpr uint8_t max_count_of_bones_per_vrtx = 4;
		static inline constexpr uint16_t max_count_of_bones = 256;

		glm::vec3 pos={0.f,0.f,0.f};
		glm::vec3 normal={0.f,0.f,0.f};
		glm::vec2 texCoord={0.f,0.f};
		uint32_t bone_IDs[max_count_of_bones_per_vrtx] = {0,0,0,0};
		float bone_weights[max_count_of_bones_per_vrtx]={0,0,0,0};

		static vk::VertexInputBindingDescription getBindingDescription()
		{
			vk::VertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(BonedVertex);
			bindingDescription.inputRate = vk::VertexInputRate::eVertex;

			return bindingDescription;
		}

		constexpr static std::array<vk::VertexInputAttributeDescription, 5> getAttributeDescriptions()
		{
			std::array<vk::VertexInputAttributeDescription, 5> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
			attributeDescriptions[0].offset = offsetof(BonedVertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
			attributeDescriptions[1].offset = offsetof(BonedVertex, normal);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
			attributeDescriptions[2].offset = offsetof(BonedVertex, texCoord);

			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = vk::Format::eR32G32B32A32Uint;
			attributeDescriptions[3].offset = offsetof(BonedVertex, bone_IDs);

			attributeDescriptions[4].binding = 0;
			attributeDescriptions[4].location = 4;
			attributeDescriptions[4].format = vk::Format::eR32G32B32A32Sfloat;
			attributeDescriptions[4].offset = offsetof(BonedVertex, bone_weights);

			return attributeDescriptions;
		}
	};
}
