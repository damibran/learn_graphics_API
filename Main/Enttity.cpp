#include <stack>
#include "Enttity.h"
#include "Componenets/RelationshipComponent.h"
#include "Componenets/SkeletalModelComponent.h"
#include "Componenets/AnimationComponent.h"

namespace dmbrn
{
	Enttity::Enttity(entt::registry& registry, const std::string& name):
		registry_(&registry),
		entityID_(registry_->create())
	{
		registry_->emplace<TagComponent>(entityID_, name);
		registry_->emplace<TransformComponent>(entityID_);
		registry_->emplace<RelationshipComponent>(entityID_, registry);
	}

	Enttity::Enttity(entt::registry& registry, const std::string& name, Enttity parent):
		Enttity(registry, name)
	{
		RelationshipComponent& this_rc = getComponent<RelationshipComponent>();
		this_rc.parent = parent;

		RelationshipComponent& parent_rc = parent.getComponent<RelationshipComponent>();

		Enttity old_first = parent_rc.first;

		if (old_first)
		{
			old_first.getComponent<RelationshipComponent>().prev = *this;
		}

		parent_rc.first = *this;
		this_rc.next = old_first;
	}

	Enttity::child_iterator::child_iterator(Enttity ent): val(ent)
	{
		stack.push(Enttity{});

		Enttity child = ent;

		while (child)
		{
			stack.push(child);
			child = child.getComponent<RelationshipComponent>().next;
		}

		val = stack.top();
		stack.pop();
	}

	Enttity::child_iterator& Enttity::child_iterator::operator++()
	{
		Enttity child = val.getComponent<RelationshipComponent>().first;

		while (child)
		{
			stack.push(child);
			child = child.getComponent<RelationshipComponent>().next;
		}

		val = stack.top();
		stack.pop();

		return *this;
	}

	void Enttity::markTransformAsEdited(uint32_t frame)const
	// we make it edited for this node and dirty all the way to the root
	{
		RelationshipComponent& this_rc = getComponent<RelationshipComponent>();
		TransformComponent& this_tc = getComponent<TransformComponent>();
		this_tc.markAsEdited();
		if (this_rc.parent)
		{
			this_rc.parent.markTransformAsDirty(frame);
		}
	}

	std::vector<Enttity> Enttity::getVectorOfAllChild() const
	{
		std::vector<Enttity> res;
		std::stack<Enttity> stack;

		Enttity this_child = getComponent<RelationshipComponent>().first;

		while (this_child)
		{
			stack.push(this_child);
			this_child = this_child.getComponent<RelationshipComponent>().next;
		}

		while (!stack.empty())
		{
			Enttity ent = stack.top();
			res.push_back(ent);
			stack.pop();
			Enttity child = ent.getComponent<RelationshipComponent>().first;
			while (child)
			{
				stack.push(child);
				child = child.getComponent<RelationshipComponent>().next;
			}
		}

		return res;
	}

	Enttity Enttity::findRecordingAnimationCompParent()
	{
		AnimationComponent* this_a_c = tryGetComponent<AnimationComponent>();
		if (this_a_c && this_a_c->is_recording)
			return *this;
		else
		{
			RelationshipComponent& r_c = getComponent<RelationshipComponent>();
			if (r_c.parent)
			{
				return r_c.parent.findRecordingAnimationCompParent();
			}
			else
				return Enttity{};
		}
	}

	void Enttity::markTransformAsDirty(uint32_t frame) // we make it dirty all the way to the root
	{
		TransformComponent& this_tc = getComponent<TransformComponent>();

		if (!this_tc.isDirtyForAllFrames())
		// means that one tree branch was edited simultaneously (during one frame) e.g. by animation
		{
			RelationshipComponent& this_rc = getComponent<RelationshipComponent>();
			this_tc.markAsDirty();
			if (this_rc.parent)
			{
				this_rc.parent.markTransformAsDirty(frame);
			}
		}
	}
}
