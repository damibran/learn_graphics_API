#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

namespace dmbrn
{
	constexpr uint8_t max_count_of_bones_per_vrtx = 4;
	constexpr uint16_t max_count_of_bones = 256; // 8-bit bone id

	struct BonedVertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 texCoord;
		uint8_t bone_IDs[max_count_of_bones_per_vrtx];
		float bone_weights[max_count_of_bones_per_vrtx];

		static vk::VertexInputBindingDescription getBindingDescription()
		{
			vk::VertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(BonedVertex);
			bindingDescription.inputRate = vk::VertexInputRate::eVertex;

			return bindingDescription;
		}

		static std::array<vk::VertexInputAttributeDescription, 5> getAttributeDescriptions()
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
			attributeDescriptions[3].format = vk::Format::eR8G8B8A8Uint;
			attributeDescriptions[3].offset = offsetof(BonedVertex, bone_IDs);

			attributeDescriptions[4].binding = 0;
			attributeDescriptions[4].location = 4;
			attributeDescriptions[4].format = vk::Format::eR32G32B32A32Sfloat;
			attributeDescriptions[4].offset = offsetof(BonedVertex, bone_weights);

			return attributeDescriptions;
		}
	};
}
