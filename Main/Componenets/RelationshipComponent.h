#pragma once
#include "Main/Enttity.h"


namespace dmbrn
{
	/**
	 * \brief describes hierarchical relations between entities
	 */
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
