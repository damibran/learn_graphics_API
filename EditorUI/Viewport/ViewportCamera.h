#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "CameraRenderData.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

#include "ECS/Componenets/TransformComponent.h"
#include "ECS/Componenets/CameraComponent.h"

namespace dmbrn
{
	/**
	 * \brief represents camera of viewport
	 */
	class ViewportCamera
	{
		friend class Viewport;
	public:
		const ImGuiMouseButton pan_button = ImGuiMouseButton_Right;

		ViewportCamera(ImVec2 size)
			: transform_({0, 7, 0}, {90, 0, -180}),
			  camera_comp(size)
		{
		}

		void update(double delta_t)
		{
			ImGuiContext& g = *GImGui;
			auto window = ImGui::GetCurrentWindow();

			if (!ImGui::IsWindowFocused()&& ImGui::IsWindowHovered() && ImGui::IsMouseClicked(pan_button) && window->ContentRegionRect.Contains(
				g.IO.MousePos))
				ImGui::SetWindowFocus();

			if (ImGui::IsWindowFocused() && ImGui::IsMouseDown(pan_button) && window->ContentRegionRect.Contains(
				g.IO.MouseClickedPos[pan_button]))
			{
				glm::vec3 cam_move_dir{0};

				if (ImGui::IsKeyDown(ImGuiKey_A))
					cam_move_dir.x -= 1;
				else if (ImGui::IsKeyDown(ImGuiKey_D))
					cam_move_dir.x += 1;

				if (ImGui::IsKeyDown(ImGuiKey_S))
					cam_move_dir.z += 1;
				else if (ImGui::IsKeyDown(ImGuiKey_W))
					cam_move_dir.z -= 1;

				if (ImGui::IsKeyDown(ImGuiKey_E))
					cam_move_dir.y += 1;
				else if (ImGui::IsKeyDown(ImGuiKey_Q))
					cam_move_dir.y -= 1;

				moveCamera(cam_move_dir, delta_t);

				if (ImGui::IsMouseDragging(pan_button))
				{
					ImVec2 new_mouse_delt = ImGui::GetMouseDragDelta(pan_button);
					ImVec2 mouse_rot = new_mouse_delt - last_mouse_delt;

					last_mouse_delt = new_mouse_delt;

					rotateCamera({-mouse_rot.x, -mouse_rot.y});
				}
			}

			if (ImGui::IsMouseReleased(pan_button))
			{
				last_mouse_delt = {0, 0};
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
			renderer_data_.update(frame, getViewMat(), camera_comp.getMatrix());
		}

		void bindData(int frame, const vk::raii::CommandBuffer& command_buffer) const
		{
			renderer_data_.bind(frame, command_buffer);
		}

	private:
		double mouse_sensitivity_ = 0.15;
		double speed_ = 7.;
		ImVec2 last_mouse_delt = {0, 0};
		TransformComponent transform_;
		CameraComponent camera_comp;
		CameraRenderData renderer_data_;

		void moveCamera(const glm::vec3 dir, const double dt)
		{
			const float tspeed = static_cast<float>(dt * speed_);
			transform_.translate(tspeed * glm::mat3(transform_.getRotationMatrix()) * dir);
		}

		void rotateCamera(const glm::vec2& mouse_dir)
		{
			glm::vec2 offset(mouse_dir.y, mouse_dir.x);

			offset *= mouse_sensitivity_;

			glm::vec3 rot_deg(transform_.getRotationDegrees());

			rot_deg += glm::vec3(offset.x, 0, offset.y);

			const bool constrainPitch = true;

			float pitch = rot_deg.x;

			// make sure that when pitch is out of bounds, screen doesn't get flipped
			if (constrainPitch)
			{
				if (pitch > 179)
					rot_deg.x = 179;
				if (pitch < 1)
					rot_deg.x = 1;
			}

			transform_.setDegrees(rot_deg);
		}
	};
}
