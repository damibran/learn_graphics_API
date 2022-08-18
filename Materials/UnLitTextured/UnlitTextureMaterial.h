#pragma once
#include "UnLitDescriptorSets.h"
#include "Materials/UnLitTextured/UnLitDescriptorsStatics.h"
#include "Wrappers/UniformBuffers.h"
#include "Wrappers/Singletons/Singletons.h"

namespace dmbrn
{
	struct UnlitTextureMaterial
	{
		UnlitTextureMaterial(const PhysicalDevice& physical_device, const LogicalDevice& device,const UnLitDescriptorsStatics& statics):
		uniform_buffers_(physical_device,device),
		descriptor_sets_(device,statics,uniform_buffers_)
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
