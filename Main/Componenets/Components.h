#pragma once
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "MaterialSystem/ShaderEffects/ShaderEffect.h"
#include "Wrappers/Singletons/PerRenderableData.h"
#include "Wrappers/Singletons/PerSkeletonData.h"
#include "Wrappers/Mesh.h"
#include "Wrappers/SkeletalMesh.h"
#include "Main/Enttity.h"

namespace dmbrn
{
	struct TagComponent
	{
		std::string tag;

		TagComponent(const std::string& name) :
			tag(name)
		{
		}
	};

	struct RelationshipComponent
	{
		Enttity first;
		Enttity prev;
		Enttity next;
		Enttity parent;

		RelationshipComponent(entt::registry& registry): first(registry), prev(registry), next(registry),
		                                                 parent(registry)
		{
		}
	};

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

	// TODO proper destructor with unregister
	struct RenderableComponent
	{
		static inline PerRenderableData per_object_data_buffer_{Singletons::device, Singletons::physical_device};
		size_t inGPU_transform_offset;
		bool need_GPU_state_update = true;

		RenderableComponent():
			inGPU_transform_offset(per_object_data_buffer_.registerObject())
		{
		}

		~RenderableComponent()=default;
	};

	struct ModelComponent
	{
		ModelComponent()=default;

		ModelComponent(Mesh&& mesh, ShaderEffect* shader = nullptr) :
			mesh(std::move(mesh)),
			shader_(shader)
		{
		}

		ShaderEffect* getShader()
		{
			return shader_;
		}

		Mesh mesh;
		ShaderEffect* shader_ = nullptr;
	};

	// TODO proper unregister
	struct SkeletalModelComponent
	{
		static inline PerSkeletonData per_skeleton_data_{Singletons::device, Singletons::physical_device,RenderableComponent::per_object_data_buffer_};
		SkeletalModelComponent()=default;

		SkeletalModelComponent(SkeletalMesh&& mesh, ShaderEffect* shader = nullptr) :
			in_GPU_mtxs_offset(per_skeleton_data_.registerObject()),
			mesh(std::move(mesh)),
			shader_(shader)
		{
		}

		ShaderEffect* getShader()
		{
			return shader_;
		}

		size_t in_GPU_mtxs_offset;
		SkeletalMesh mesh;
		ShaderEffect* shader_ = nullptr;
	};

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
