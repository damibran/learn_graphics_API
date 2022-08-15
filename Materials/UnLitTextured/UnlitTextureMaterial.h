#pragma once
#include "UnLitDescriptorSets.h"
#include "UnLitGraphicsPipeline.h"
#include "Wrappers/UniformBuffers.h"
#include "Wrappers/Singletons/Singletons.h"

namespace dmbrn
{
	struct UnlitTextureMaterial
	{
		UnlitTextureMaterial(const Singletons& singletons,const ViewportRenderPass& render_pass):
		uniform_buffers_(singletons.physical_device,singletons.device),
		descriptor_sets_(singletons.device,singletons.un_lit_descriptor_statics,uniform_buffers_),
		graphics_pipeline_(singletons.device,render_pass,singletons.un_lit_descriptor_statics)
		{
		}

		UniformBuffers uniform_buffers_;
		UnLitDescriptorSets descriptor_sets_;
		UnLitGraphicsPipeline graphics_pipeline_;
	};
}
