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

#include "Wrappers/Model.h"

namespace dmbrn
{
	class Component
	{
		virtual ~Component() = default;
		virtual void drawToInspector() =0;
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
		                   glm::vec3 scale = {1, 1, 1}):
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

	class ModelComponent
	{
	public:
		ModelComponent(const std::string& path):
			model_(&(*Model::model_instances.emplace(path, path).first).second)
		{
		}

		void draw(int frame, const vk::raii::CommandBuffer& command_buffers, glm::mat4 modelMat, const glm::mat4& view,
		          const glm::mat4& proj)
		{
			model_->draw(frame, command_buffers, modelMat, view, proj);
		}

		void setNewModel(const std::string& path)
		{
			model_ = &(*Model::model_instances.emplace(path, path).first).second;
		}

		const Model* getModel()
		{
			return model_;
		}

	private:
		Model* model_;
	};

	class CameraComponent
	{
	public:
		glm::mat4 proj_;

		CameraComponent(TransformComponent& cam_transform, ImVec2 size):
			transform_(cam_transform)
		{
			proj_ = glm::perspective(glm::radians(45.0f),
			                         size.x / size.y, 0.1f, 500.0f);
		}

		void changeAspect(ImVec2 size)
		{
			proj_ = glm::perspective(glm::radians(45.0f),
			                         size.x / size.y, 0.1f, 500.0f);
		}

		void update(float delta_t)
		{
			if (ImGui::IsWindowFocused())
			{
				glm::vec3 cam_move_dir{0};

				if (ImGui::IsKeyDown(ImGuiKey_A))
					cam_move_dir.x -= 1;
				else if (ImGui::IsKeyDown(ImGuiKey_D))
					cam_move_dir.x += 1;

				if (ImGui::IsKeyDown(ImGuiKey_S))
					cam_move_dir.z -= 1;
				else if (ImGui::IsKeyDown(ImGuiKey_W))
					cam_move_dir.z += 1;

				if (ImGui::IsKeyDown(ImGuiKey_E))
					cam_move_dir.y -= 1;
				else if (ImGui::IsKeyDown(ImGuiKey_Q))
					cam_move_dir.y += 1;

				moveCamera(cam_move_dir, delta_t);

				if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
				{
					ImVec2 new_mouse_delt = ImGui::GetMouseDragDelta();
					ImVec2 mouse_rot = new_mouse_delt - last_mouse_delt;

					last_mouse_delt = new_mouse_delt;

					rotateCamera({mouse_rot.x, -mouse_rot.y});
				}

				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					last_mouse_delt = {0, 0};
				}
			}
		}

		glm::mat4 getViewMat()
		{
			return inverse(transform_.getMatrix());
		}

	private:
		TransformComponent& transform_;
		float mouse_sensitivity_ = 0.15f;
		float speed_ = 7;
		ImVec2 last_mouse_delt = {0, 0};

		void moveCamera(const glm::vec3 dir, const float dt)
		{
			const float tspeed = dt * speed_;
			transform_.translate(tspeed * glm::mat3(transform_.getRotationMatrix()) * dir);
		}

		void rotateCamera(const glm::vec2 mouse_dir)
		{
			glm::vec2 offset(mouse_dir.y, mouse_dir.x);

			offset *= mouse_sensitivity_;

			glm::vec3 t(transform_.getRotationDegrees());

			t += glm::vec3(offset, 0.);

			transform_.rotation = t;

			const bool constrainPitch = true;

			float pitch = transform_.getRotationDegrees().x;

			// make sure that when pitch is out of bounds, screen doesn't get flipped
			if (constrainPitch)
			{
				if (pitch > 89.0f)
					transform_.rotation.x = 89.0f;
				if (pitch < -89.0f)
					transform_.rotation.x = -89.0f;
			}
		}
	};
}
