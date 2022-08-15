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

		void updateUBO(int curentFrame, glm::mat4 modelMat)
		{
			const float speed = 90;

			UniformBuffers::UniformBufferObject ubo{};
			ubo.model = modelMat;
			ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
			                  glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.proj = glm::perspective(glm::radians(45.0f),
			                            1.0f, 0.1f,
			                            10.0f);
			ubo.proj[1][1] *= -1;

			void* data = uniform_buffers_.getUBMemory(curentFrame).mapMemory(0, sizeof(ubo));
			memcpy(data, &ubo, sizeof(ubo));
			uniform_buffers_.getUBMemory(curentFrame).unmapMemory();
		}

		UniformBuffers uniform_buffers_;
		UnLitDescriptorSets descriptor_sets_;
		UnLitGraphicsPipeline graphics_pipeline_;
	};
}
