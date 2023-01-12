#pragma once

#include "Materials/UnLitTextured/UnlitTexturedMaterial.h"
#include <EditorUI/Viewport/ViewportCamera.h>


namespace dmbrn
{
	class Renderer
	{
	public:

		static void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			UnlitTexturedMaterial::setRenderPass(render_pass);
		}

		static void newView(int frame, ViewportCamera& viewport_camera, const vk::raii::CommandBuffer& command_buffer)
		{
			viewport_camera.updateRenderData(frame);
			viewport_camera.bindData(frame, command_buffer);
		}
	};
}
