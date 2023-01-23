#pragma once
#include<entt/entt.hpp>

#include"Main/Componenets/Components.h"

namespace dmbrn
{
	class Enttity
	{
	public:

		Enttity(const Enttity&)=delete;

		Enttity(entt::registry& registry):
			registry_(&registry)
		{
		}

		Enttity(entt::registry& registry, const std::string& name):
			registry_(&registry)
		{
			registry_->emplace<TagComponent>(entityID_, name);
			registry_->emplace<TransformComponent>(entityID_);
		}

		Enttity(entt::registry& registry, const std::string& name, Enttity& parent):
			registry_(&registry),
			entityID_(registry_->create())
		{
			registry_->emplace<TagComponent>(entityID_, name);
			registry_->emplace<TransformComponent>(entityID_);

			addComponent<RelationshipComponent>();
			auto& cur_comp = getComponent<RelationshipComponent>();

			auto& parent_comp = parent.getComponent<RelationshipComponent>();

			auto old_first = parent_comp.first;

			if (old_first != entt::null)
			{
				registry.get<RelationshipComponent>(old_first).prev = *this;
			}

			parent_comp.first = *this;
			cur_comp.next = old_first;
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

		operator uint32_t() const { return static_cast<uint32_t>(entityID_); }
		operator entt::entity() const { return entityID_; };

	private:
		entt::registry* registry_;
		entt::entity entityID_{entt::null};

		void updateFirstPtr(Enttity& node, const Enttity& new_first)
		{
			auto& curr_comp = node.getComponent<RelationshipComponent>();
			entt::entity curr_ind = curr_comp.first;

			while (curr_ind != entt::null)
			{
				curr_comp.first = new_first;

				curr_ind = curr_comp.next;
				curr_comp = registry_->get<RelationshipComponent>(curr_ind);
			}
		}
	};
}
