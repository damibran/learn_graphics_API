#pragma once
#include<entt/entt.hpp>

#include "Main/Componenets/ModelComponent.h"
#include "Main/Componenets/SkeletalModelComponent.h"
#include "Componenets/TagComponent.h"

namespace dmbrn
{
	class Enttity
	{
	public:
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
			static_assert(!std::is_same_v<Type, ModelComponent>);
			static_assert(!std::is_same_v<Type, SkeletalModelComponent>);

			registry_->emplace<Type>(entityID_, std::forward<Args>(args)...);
		}

		void addModelComponent(Mesh&& mesh, ShaderEffect* shader = nullptr);

		void addSkeletalModelComponent(SkeletalMesh&& mesh, ShaderEffect* shader = nullptr);

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
		//operator entt::entity() const { return entityID_; }

	private:
		entt::registry* registry_ = nullptr;
		entt::entity entityID_{entt::null};

		void markTransformAsDirty(uint32_t frame);
	};
}
