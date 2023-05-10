#pragma once

#include "MaterialSystem/ShaderEffects/UnLitTextured/UnlitTexturedShaderEffect.h"
#include "MaterialSystem/ShaderEffects/Outline/UnlitTexturedOutlinedShaderEffect.h"
#include <EditorUI/Viewport/ViewportCamera.h>
#include "PerSkeletonData.h"


namespace dmbrn
{
	class Renderer
	{
	public:

		static void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			UnlitTexturedShaderEffect::setRenderPass(render_pass);
			UnlitTexturedOutlinedShaderEffect::setRenderPass(render_pass);
		}

		static void newView(int frame, ViewportCamera& viewport_camera, const vk::raii::CommandBuffer& command_buffer)
		{
			viewport_camera.updateRenderData(frame);
			viewport_camera.bindData(frame, command_buffer);
		}

		static inline PerStaticModelData per_static_data_buffer_{Singletons::device, Singletons::physical_device};
		static inline PerSkeletonData per_skeleton_data_{Singletons::device, Singletons::physical_device};

		static inline UnlitTexturedShaderEffect un_lit_textured{per_static_data_buffer_,per_skeleton_data_};
		static inline UnlitTexturedOutlinedShaderEffect outlined_{per_static_data_buffer_,per_skeleton_data_};
	};
}
