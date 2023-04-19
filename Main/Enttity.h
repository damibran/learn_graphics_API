#pragma once
#include<entt/entt.hpp>

#include "Main/Componenets/ModelComponent.h"
#include "Componenets/TagComponent.h"

namespace dmbrn
{
	struct SkeletalModelComponent;

	class Enttity
	{
	public:
		Enttity() = default;

		Enttity(Enttity&& other): registry_(other.registry_), entityID_(other.entityID_)
		{
		}

		Enttity(entt::registry& registry):
			registry_(&registry)
		{
		}

		Enttity(entt::registry& registry, const std::string& name);

		Enttity(entt::registry& registry, const std::string& name, Enttity parent);

		Enttity(entt::registry& registry, entt::entity entityID):
			registry_(&registry),
			entityID_(entityID)
		{
		}

		Enttity(const Enttity& other)
		{
			registry_ = other.registry_;
			entityID_ = other.entityID_;
		}

		void destroy()
		{
			registry_->destroy(entityID_);
		}

		void operator=(const Enttity& other)
		{
			registry_ = other.registry_;
			entityID_ = other.entityID_;
		}

		bool operator==(const Enttity& other)const
		{
			return entityID_ == other.entityID_ && registry_ == other.registry_;
		}

		template <typename... Type>
		[[nodiscard]] decltype(auto) getComponent()
		{
			if constexpr (sizeof...(Type) == 1u)
			{
				return (const_cast<Type&>(registry_->get<Type>(entityID_)), ...);
			}
			else
			{
				return std::forward_as_tuple(registry_->get<Type>(entityID_)...);
			}
		}

		template <typename... Type>
		[[nodiscard]] decltype(auto) getComponent() const
		{
			if constexpr (sizeof...(Type) == 1u)
			{
				return (const_cast<Type&>(registry_->get<Type>(entityID_)), ...);
			}
			else
			{
				return std::forward_as_tuple(registry_->get<Type>(entityID_)...);
			}
		}

		template <typename Type, typename ... Args>
		void addComponent(Args&&... args)
		{
			registry_->emplace<Type>(entityID_, std::forward<Args>(args)...);
		}

		template <typename T>
		T* tryGetComponent()
		{
			return registry_->try_get<T>(entityID_);
		}

		template <typename T>
		const T* tryGetComponent() const
		{
			return registry_->try_get<T>(entityID_);
		}

		operator bool() const
		{
			return entityID_ != entt::null;
		}

		void markTransformAsEdited(uint32_t frame);

		operator uint32_t() const { return static_cast<uint32_t>(entityID_); }

	private:
		entt::registry* registry_ = nullptr;
		entt::entity entityID_{entt::null};

		void markTransformAsDirty(uint32_t frame);
	};
}
