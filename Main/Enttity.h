#pragma once
#include<entt/entt.hpp>

#include"Main/Componenets/Components.h"

namespace dmbrn
{
	class Enttity
	{
	public:
		Enttity(entt::registry& registry):
		registry_(&registry)
		{
		}

		Enttity(entt::registry& registry, const std::string& name):
			registry_(&registry)
		{
			entityID_ = registry_->create();
			registry_->emplace<TagComponent>(entityID_, name);
			registry_->emplace<TransformComponent>(entityID_);
		}

		Enttity(entt::registry& registry, entt::entity entityID):
			registry_(&registry),
			entityID_(entityID)
		{
		}

		void operator=(const Enttity& other)
		{
			registry_ = other.registry_;
			entityID_ = other.entityID_;
		}

		uint32_t getId() const
		{
			return static_cast<uint32_t>(entityID_);
		}

		bool operator==(const Enttity& other)
		{
			return entityID_ == other.entityID_ && registry_ == other.registry_;
		}

		template <class T>
		T& getComponent()
		{
			return registry_->get<T>(entityID_);
		}

		template <class T>
		const T& getComponent() const
		{
			return registry_->get<T>(entityID_);
		}

		template <typename Type, typename... Args>
		void addComponent(Args&&...args)
		{
			registry_->emplace<Type>(entityID_, std::forward<Args>(args)...);
		}

		template <typename T>
		T* tryGetComponent()
		{
			return registry_->try_get<T>(entityID_);
		}

		operator bool() const
		{
			return entityID_ != entt::null;
		}

	private:
		entt::registry* registry_;
		entt::entity entityID_{entt::null};
	};
}
