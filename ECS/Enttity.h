#pragma once
#include <stack>
#include<entt/entt.hpp>

#include "ECS/Componenets/StaticModelComponent.h"
#include "Componenets/TagComponent.h"

namespace dmbrn
{
	struct SkeletalModelComponent;

	/**
	 * \brief is wrapper for entt::entity identificator for convenience
	 */
	class Enttity
	{
	public:
		struct child_iterator;

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

		bool operator==(const Enttity& other) const
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

		entt::entity getId()const
		{
			return entityID_;
		}

		void markTransformAsEdited(uint32_t frame)const;
		std::vector<Enttity> getVectorOfAllChild() const;

		operator uint32_t() const { return static_cast<uint32_t>(entityID_); }
		Enttity findRecordingAnimationCompParent();

		struct hash
		{
			size_t operator()(const Enttity& ent) const
			{
				return static_cast<uintptr_t>(ent.entityID_) ^ reinterpret_cast<uintptr_t>(ent.registry_);
			}
		};

	private:
		entt::registry* registry_ = nullptr;
		entt::entity entityID_{entt::null};

		void markTransformAsDirty(uint32_t frame);
	};

	// TODO actually this is only forward iterator
	struct Enttity::child_iterator
	{
		child_iterator(Enttity ent);

		child_iterator& operator++();

		bool operator==(const child_iterator& other)
		{
			return val == other.val;
		}

		bool operator!=(const child_iterator& other)
		{
			return !(*this == other);
		}

		Enttity& operator*()
		{
			return val;
		}

		Enttity* operator->()
		{
			return &val;
		}

	private:
		Enttity val;
		std::stack<Enttity> stack;
	};
}
