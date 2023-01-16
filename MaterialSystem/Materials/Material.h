#pragma once

namespace dmbrn
{
	class Material
	{
	public:
		virtual void bindMaterialData(int frame, const vk::raii::CommandBuffer& command_buffer, vk::PipelineLayout layout)const = 0;
	};
}