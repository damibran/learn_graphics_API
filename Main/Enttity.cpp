#include "Enttity.h"
#include "Componenets/Components.h"

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

	void Enttity::addModelComponent(Mesh&& mesh, ShaderEffect* shader)
	{
		addComponent<RenderableComponent>();
		registry_->emplace<ModelComponent>(entityID_,std::move(mesh), shader);
	}

	void Enttity::addSkeletalModelComponent(SkeletalMesh&& mesh, ShaderEffect* shader)
	{
		addComponent<RenderableComponent>();
		registry_->emplace<SkeletalModelComponent>(entityID_,std::move(mesh), shader);
	}

	void Enttity::markTransformAsEdited(uint32_t frame)
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

	void Enttity::markTransformAsDirty(uint32_t frame) // we make it dirty all the way to the root
	{
		TransformComponent& this_tc = getComponent<TransformComponent>();

		//if (!this_tc.isDirtyForFrame(frame))
		//{
		RelationshipComponent& this_rc = getComponent<RelationshipComponent>();
		this_tc.markAsDirty();
		if (this_rc.parent)
		{
			this_rc.parent.markTransformAsDirty(frame);
		}
		//}
	}
}