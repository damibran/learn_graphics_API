#pragma once
#include "Wrappers/Singletons/Renderer.h"
#include "UnLitDescriptorSets.h"
#include "UnLitGraphicsPipeline.h"
#include "Wrappers/UniformBuffers.h"
#include "Wrappers/Singletons/Singletons.h"

namespace dmbrn
{
	struct UnlitTextureMaterial
	{
		UnlitTextureMaterial():
		uniform_buffers_(Singletons::physical_device,Singletons::device),
		descriptor_sets_(Singletons::device,Renderer::un_lit_descriptors_statics_,uniform_buffers_)
		{
		}

		void updateUBO(int curentFrame, glm::mat4 modelMat,const glm::mat4& view,const glm::mat4& proj)
		{
			const float speed = 90;

			UniformBuffers::UniformBufferObject ubo{};
			ubo.model = modelMat;
			ubo.view = view;
			ubo.proj = proj;

			ubo.proj[1][1] *= -1;

			void* data = uniform_buffers_.getUBMemory(curentFrame).mapMemory(0, sizeof(ubo));
			memcpy(data, &ubo, sizeof(ubo));
			uniform_buffers_.getUBMemory(curentFrame).unmapMemory();
		}

		UniformBuffers uniform_buffers_;
		UnLitDescriptorSets descriptor_sets_;
	};
}
