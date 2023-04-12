#pragma once

#include "Wrappers/Singletons/LogicalDevice.h"
#include <glm/glm.hpp>
#include "glm/ext/matrix_transform.hpp"

namespace dmbrn
{
	struct TransformComponent
	{
		glm::vec3 position;
		glm::vec3 rotation; // pitch ,yaw, roll in degrees
		glm::vec3 scale;

		std::array<bool, LogicalDevice::MAX_FRAMES_IN_FLIGHT> dirty;
		std::array<bool, LogicalDevice::MAX_FRAMES_IN_FLIGHT> edited;

		glm::mat4 globalTransformMatrix = glm::mat4(1.0f); // hierarchy transform without self trans matrix

		TransformComponent(glm::vec3 pos = {0, 0, 0},
		                   glm::vec3 rot = {0, 0, 0},
		                   glm::vec3 scale = {1, 1, 1}) :
			position(pos),
			rotation(rot),
			scale(scale)
		{
			for (size_t i = 0; i < LogicalDevice::MAX_FRAMES_IN_FLIGHT; ++i)
			{
				edited[i] = true;
				dirty[i] = false;
			}
		}

		[[nodiscard]] glm::mat4 getRotationMatrix() const
		{
			return glm::rotate(glm::mat4{1}, glm::radians(rotation.z), {0, 0, 1}) *
				glm::rotate(glm::mat4{1}, glm::radians(rotation.y), {0, 1, 0}) *
				glm::rotate(glm::mat4{1}, glm::radians(rotation.x), {1, 0, 0});
		}

		glm::mat4 getMatrix() const
		{
			return glm::translate(glm::mat4(1), position) * getRotationMatrix() *
				glm::scale(glm::mat4(1), scale);
		}

		void translate(const glm::vec3& v)
		{
			position = glm::translate(glm::mat4(1), v) * glm::vec4(position, 1);
		}

		void rotate(const glm::vec3& r)
		{
			rotation += r;
		}

		glm::vec3 getRotationDegrees() const
		{
			return rotation;
		}

		void markAsEdited()
		{
			for (int i = 0; i < LogicalDevice::MAX_FRAMES_IN_FLIGHT; ++i)
				edited[i] = true;
		}

		void markAsDirty()
		{
			for (int i = 0; i < LogicalDevice::MAX_FRAMES_IN_FLIGHT; ++i)
				dirty[i] = true;
		}

		bool isDirtyForFrame(uint32_t frame) const
		{
			return dirty[frame];
		}

		bool isEditedForFrame(uint32_t frame) const
		{
			return edited[frame];
		}
	};
}
