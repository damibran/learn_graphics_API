#pragma once
#include <functional>

namespace dmbrn
{
	class Script
	{
	public:
		Script()=default;
		Script(std::function<void(float)> func):
		function_(std::move(func))
		{}
		virtual void update(float delta_t)
		{
			function_(delta_t);
		}
	private:
		std::function<void(float)> function_;
	};
}
