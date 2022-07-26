#pragma once
#include"Shape3D.h"
#include"../Wrappers/Model.h"

namespace dmbrn
{
	class VisualShape :public Shape3D
	{
	public:
		VisualShape(std::string ModelPath):
			model_()
		{

		}
	private:
		Model model_;
	};
}