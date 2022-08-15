#pragma once
#include<entt/entt.hpp>

#include"Main/Componenets/Components.h"

namespace dmbrn
{
	class Enttity
	{
	public:
		Enttity(entt::registry& registry, const std::string& name):
			registry_(registry)
		{
			entityID_ = registry_.create();
			registry_.emplace<TagComponent>(entityID_, name);
			registry_.emplace<TransformComponent>(entityID_);
		}

		~Enttity()
		{
			registry_.destroy(entityID_);
		}

		template <class T>
		T& getComponent()
		{
			return registry_.get<T>(entityID_);
		}

		template <typename Type, typename... Args>
		void addComponent(Args&&...args)
		{
			registry_.emplace<Type>(entityID_,std::forward<Args>(args)...);
		}

	private:
		entt::registry& registry_;
		entt::entity entityID_;
	};
}
