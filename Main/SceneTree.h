#pragma once

#include "Shape.h"

namespace dmbrn
{
	class SceneTree
	{
	public:
		SceneTree(const Singletons& singletons)
		{
		}


		void draw()
		{
			for (auto& vi : root)
			{
				vi.draw();
			}
		}

	private:
		std::vector<Shape> root;
	};
}
