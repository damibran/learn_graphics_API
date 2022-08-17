#pragma once

#include "Wrappers/Singletons/Singletons.h"
#include "Materials/UnLitTextured/UnLitDescriptorsStatics.h"

namespace dmbrn
{
	class Renderer
	{
	public:
		void setRenderPass(vk::raii::RenderPass* render_pass)
		{
			
		}
	private:
		static inline vk::raii::RenderPass* render_pass_{nullptr};
		static inline UnLitDescriptorsStatics un_lit_descriptors_statics_;
	};
}
