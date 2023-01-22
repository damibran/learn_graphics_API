#pragma once
#include <string>
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "MaterialSystem/ShaderEffects/ShaderEffect.h"
#include "Wrappers/Singletons/PerObjectDataBuffer.h"
#include "Wrappers/Mesh.h"


namespace dmbrn
{
	class Component
	{
		virtual ~Component() = default;
		virtual void drawToInspector() = 0;
	};

	struct TagComponent
	{
		std::string tag;

		TagComponent(const std::string& name) :
			tag(name)
		{
		}
	};

	struct TransformComponent
	{
		glm::vec3 position;
		glm::vec3 rotation; // pitch ,yaw, roll in degrees
		glm::vec3 scale;

		TransformComponent(glm::vec3 pos = {0, 0, 0},
		                   glm::vec3 rot = {0, 0, 0},
		                   glm::vec3 scale = {1, 1, 1}) :
			position(pos),
			rotation(rot),
			scale(scale)
		{
		}

		[[nodiscard]] glm::mat4 getRotationMatrix() const
		{
			return orientate4(glm::vec3{glm::radians(rotation.x), glm::radians(rotation.z), glm::radians(rotation.y)});
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
	};

	struct ModelComponent
	{
		static inline PerObjectDataBuffer per_object_data_buffer_{Singletons::device, Singletons::physical_device};

		ModelComponent()=default;

		ModelComponent(Mesh* mesh, ShaderEffect* shader = nullptr) :
			mesh(mesh),
			shader_(shader),
			inGPU_transform_offset(per_object_data_buffer_.registerObject())
		{

		}

		ShaderEffect* getShader()
		{
			return shader_;
		}

		Mesh* mesh = nullptr;
		ShaderEffect* shader_ = nullptr;
		size_t inGPU_transform_offset;
	};

	class CameraComponent
	{
	public:
		CameraComponent(ImVec2 size)
		{
			proj_ = glm::perspective(glm::radians(45.0f),
			                         size.x / size.y, 0.1f, 500.0f);
		}

		void changeAspect(ImVec2 size)
		{
			proj_ = glm::perspective(glm::radians(45.0f),
			                         size.x / size.y, 0.1f, 500.0f);
		}

		glm::mat4 getMatrix() const
		{
			return proj_;
		}

	private:
		glm::mat4 proj_;
	};
}
