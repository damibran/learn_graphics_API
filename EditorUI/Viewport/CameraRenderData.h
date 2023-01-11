#pragma once
#include <Wrappers/Singletons/Singletons.h>
#include <Wrappers/CameraUniformBuffer.h>

namespace dmbrn
{
	class CameraRenderData
	{
	public:
		CameraRenderData():
			uniform_buffer_(Singletons::physical_device,Singletons::device)
		{}
	private:
		CameraUniformBuffer uniform_buffer_;
	};
}
