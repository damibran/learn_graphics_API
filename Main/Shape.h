#pragma once
#include "Utils/Transform.h"
#include "Wrappers/Singletons/Singletons.h"
#include"../Wrappers/Model.h"
#include "Script.h"

namespace dmbrn
{
	class Shape
	{
	public:
		Shape(const Singletons& singletons,std::string model_path):
			model_(std::make_unique<Model>(model_path,singletons))
		{
		}

		void draw()
		{
			//model_->Draw()
			//
			//for (auto& child: children_)
			//{
			//	
			//}
		}

	private:
		std::vector<std::unique_ptr<Shape>> children_;
		Transform transform_;
		std::unique_ptr<Model> model_;
		std::unique_ptr<Script> script_;
	};
}