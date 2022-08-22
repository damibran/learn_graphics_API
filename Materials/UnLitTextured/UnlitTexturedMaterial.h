#pragma once
#include "UnLitTexturedDescriptorSets.h"
#include "Materials/UnLitTextured/UnLitTexturedDescriptorsStatics.h"
#include "Wrappers/UniformBuffers.h"

namespace dmbrn
{
	struct UnlitTexturedMaterial
	{
		UnlitTexturedMaterial(UnlitTexturedMaterial&& other)=default;
		UnlitTexturedMaterial& operator=(UnlitTexturedMaterial&& other)=default;

		UnlitTexturedMaterial(const UnlitTexturedMaterial& other)=delete;
		UnlitTexturedMaterial& operator=(const UnlitTexturedMaterial& other)=delete;

		UnlitTexturedMaterial(const PhysicalDevice& physical_device, const LogicalDevice& device,const UnLitTexturedDescriptorsStatics& statics):
		uniform_buffers_(physical_device,device),
		descriptor_sets_(device,statics,uniform_buffers_)
		{
		}

		void updateUBO(int curentFrame, glm::mat4 modelMat,const glm::mat4& view,const glm::mat4& proj)
		{
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
		UnLitTexturedDescriptorSets descriptor_sets_;
	};
}
