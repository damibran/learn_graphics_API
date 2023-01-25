#pragma once

#include "MaterialSystem/ShaderEffects/UnLitTextured/UnlitTexturedShaderEffect.h"
#include "MaterialSystem/ShaderEffects/Outline/UnlitTexturedOutlinedShaderEffect.h"
#include <EditorUI/Viewport/ViewportCamera.h>


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

		static inline UnlitTexturedShaderEffect un_lit_textured;
		static inline UnlitTexturedOutlinedShaderEffect outlined_;
	};
}
