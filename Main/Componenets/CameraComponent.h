#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include "imgui.h"

namespace dmbrn
{
	class CameraComponent
	{
	public:
		CameraComponent(ImVec2 size)
		{
			proj_ = glm::perspective(glm::radians(45.0f),
			                         size.x / size.y, 0.1f, 500.0f);
			proj_[1][1] *= -1;
		}

		void changeAspect(ImVec2 size)
		{
			proj_ = glm::perspective(glm::radians(45.0f),
			                         size.x / size.y, 0.1f, 500.0f);
			proj_[1][1] *= -1;
		}

		glm::mat4 getMatrix() const
		{
			return proj_;
		}

	private:
		glm::mat4 proj_;
	};
}
