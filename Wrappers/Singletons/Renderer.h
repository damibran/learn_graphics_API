#pragma once

#include "Wrappers/Singletons/Singletons.h"
#include "Materials/UnLitTextured/UnLitDescriptorsStatics.h"

namespace dmbrn
{
	class Renderer
	{
		friend class UnlitTextureMaterial;

		void drawMesh(const)
		{
			
		}

	public:
		static void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			un_lit_descriptors_statics_.setRenderPass(render_pass);
		}
	private:
		static inline UnLitDescriptorsStatics un_lit_descriptors_statics_{};
	};
}
