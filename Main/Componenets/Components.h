#pragma once
#include <string>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "Wrappers/Model.h"

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

	struct TransformComponent
	{
		glm::vec3 position;
		glm::vec3 rotation; // pitch, yaw, roll in degrees
		glm::vec3 scale;

		TransformComponent(glm::vec3 pos = {0, 0, 0},
		                   glm::vec3 rot = {0, 0, 0},
		                   glm::vec3 scale = {1, 1, 1}):
			position(pos),
			rotation(rot),
			scale(scale)
		{
		}

		glm::mat4 getMatrix() const
		{
			glm::mat4 res = glm::scale(glm::mat4(1.0f), scale);
			res = glm::rotate(res, glm::radians(rotation.x), {1, 0, 0});
			res = glm::rotate(res, glm::radians(rotation.y), {0, 1, 0});
			res = glm::rotate(res, glm::radians(rotation.z), {0, 0, 1});
			res = glm::translate(res, position);

			return res;
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
		model_(&(*(Model::model_instances.emplace(std::make_pair(path,path)).first)).second)
		{
			
		}
		void draw(int frame, const LogicalDevice& device, const vk::raii::CommandBuffer& command_buffers, const UnlitTextureMaterial& material)
		{
			model_->draw(frame,device,command_buffers,material);
		}
	private:
		Model* model_;
	};

	class CameraComponent
	{
	public:
		glm::mat4 proj_;

		CameraComponent(const bool& viewportFocused, TransformComponent& cam_transform, ImVec2 size):
			receive_input_(viewportFocused),
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
			if (receive_input_)
			{
				glm::vec3 cam_move_dir{0};

				if (ImGui::IsKeyDown(ImGuiKey_A))
					cam_move_dir.x += -1;
				else if (ImGui::IsKeyDown(ImGuiKey_D))
					cam_move_dir.x += 1;

				if (ImGui::IsKeyDown(ImGuiKey_S))
					cam_move_dir.z += -1;
				else if (ImGui::IsKeyDown(ImGuiKey_W))
					cam_move_dir.z += 1;

				if (ImGui::IsKeyDown(ImGuiKey_E))
					cam_move_dir.y += 1;
				else if (ImGui::IsKeyDown(ImGuiKey_Q))
					cam_move_dir.y += -1;

				moveCamera(cam_move_dir, delta_t);

				if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
				{
					ImVec2 new_mouse_delt = ImGui::GetMouseDragDelta();
					ImVec2 mouse_rot = new_mouse_delt - last_mouse_delt;

					last_mouse_delt = new_mouse_delt;

					rotateCamera({-mouse_rot.x, mouse_rot.y});
				}

				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					last_mouse_delt = {0, 0};
				}
			}
		}

		glm::mat4 getViewMat()
		{
			return lookAt(transform_.position, transform_.position + front_, up_);
		}

	private:
		const bool& receive_input_;
		TransformComponent& transform_;
		glm::vec3 front_ = {1.0f, 0.0f, 0.0f};
		glm::vec3 up_ = {0.0f, -1.0f, 0.0f};
		glm::vec3 right_ = {0.0f, 0, -1.0f};
		glm::vec3 world_up_ = {0, -1, 0};
		float mouse_sensitivity_ = 0.15f;
		float speed_ = 7;
		ImVec2 last_mouse_delt = {0, 0};

		void moveCamera(const glm::vec3 dir, const float dt)
		{
			const float tspeed = dt * speed_;
			const auto m = glm::mat3(right_, up_, front_);
			transform_.translate(tspeed * m * dir);
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

			// update Front, Right and Up Vectors using the updated Euler angles
			updateCameraVectors();
		}

		void updateCameraVectors()
		{
			// calculate the new Front vector
			glm::vec3 front;
			glm::vec3 rot(transform_.getRotationDegrees());
			front.x = cos(glm::radians(rot.y)) * cos(glm::radians(rot.x));
			front.y = sin(glm::radians(rot.x));
			front.z = sin(glm::radians(rot.y)) * cos(glm::radians(rot.x));
			front_ = normalize(front);
			// also re-calculate the Right and Up vector
			right_ = normalize(cross(front_, world_up_));
			// normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
			up_ = normalize(cross(right_, front_));
		}
	};
}
