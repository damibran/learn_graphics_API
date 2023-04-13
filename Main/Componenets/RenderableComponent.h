#pragma once

#include "Wrappers/Singletons/Renderer.h"

namespace dmbrn
{
	// TODO proper destructor with unregister
	struct RenderableComponent
	{
		size_t inGPU_transform_offset;
		bool need_GPU_state_update = true;

		RenderableComponent():
			inGPU_transform_offset(Renderer::per_renderable_data_buffer_.registerObject())
		{
		}

		~RenderableComponent()=default;
	};
}
