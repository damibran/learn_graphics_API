#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "CameraRenderData.h"
#include "imgui.h"

#include "Main/Componenets/Components.h"

namespace dmbrn
{
	class ViewportCamera
	{
		friend class Viewport;
	public:
		ViewportCamera(ImVec2 size)
			: transform_({0, 0, -7}),
			  camera_comp(size)
		{
		}

		void update(float delta_t)
		{
			ImGuiContext& g = *GImGui;
			auto window = ImGui::GetCurrentWindow();
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

				if (ImGui::IsWindowHovered()&&ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !window->TitleBarRect().Contains(g.IO.MousePos))
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
			return glm::inverse(transform_.getMatrix());
		}

		void updateAspectRatio(ImVec2 size)
		{
			camera_comp.changeAspect(size);
		}

		void updateRenderData(int frame)
		{
			renderer_data_.update(frame,glm::inverse(transform_.getMatrix()),camera_comp.getMatrix());
		}

		void bindData(int frame,const vk::raii::CommandBuffer& command_buffer)const
		{
			renderer_data_.bind(frame, command_buffer);
		}

	private:
		float mouse_sensitivity_ = 0.15f;
		float speed_ = 7;
		ImVec2 last_mouse_delt = {0, 0};
		TransformComponent transform_;
		CameraComponent camera_comp;
		CameraRenderData renderer_data_;
				
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
