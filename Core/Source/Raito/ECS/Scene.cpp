/*
MIT License

Copyright (c) 2023 V�ctor Falc�n Zaro

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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
		return Entity();
	}
	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		return Entity();
	}
}