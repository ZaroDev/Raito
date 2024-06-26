#include "pch.h"
#include "Scene.h"

#include "Components.h"
#include "Entity.h"


namespace Raito::ECS
{
	Entity Scene::CreateEntity(const std::string& name)
	{
		UUID uuid{};

		Entity entt = { m_Registry.create(), this };
		entt.AddComponent<IDComponent>(uuid);
		entt.AddComponent<TransformComponent>();
		auto& tag = entt.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		m_Entities[uuid] = entt;
		return entt;
	}
	void Scene::DestroyEntity(Entity entity)
	{
	}
	Entity Scene::DuplicateEntity(Entity entity)
	{
		return Entity();
	}
	Entity Scene::FindEntityByName(std::string_view name)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			const TagComponent& tc = view.get<TagComponent>(entity);
			if (tc.Tag == name)
				return Entity{ entity, this };
		}
		return {};
	}
	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		if (m_Entities.find(uuid) != m_Entities.end())
		{
			return { m_Entities.at(uuid), this };
		}

		return {};
	}
}