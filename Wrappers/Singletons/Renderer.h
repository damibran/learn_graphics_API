#pragma once

#include "Materials/UnLitTextured/UnlitTexturedMaterial.h"

namespace dmbrn
{
	class Renderer
	{
	public:

		static void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			UnlitTexturedMaterial::setRenderPass(render_pass);
		}
	};
}
