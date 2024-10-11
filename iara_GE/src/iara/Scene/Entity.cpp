#include "ir_pch.h"
#include "Entity.h"

namespace iara {

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_entity_handle{handle}, m_scene{scene}
	{
	}

}