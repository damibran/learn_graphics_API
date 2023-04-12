#pragma once
#include "Main/Enttity.h"


namespace dmbrn
{
	struct RelationshipComponent
	{
		Enttity first;
		Enttity prev;
		Enttity next;
		Enttity parent;

		RelationshipComponent(entt::registry& registry): first(registry), prev(registry), next(registry),
		                                                 parent(registry)
		{
		}
	};
}
