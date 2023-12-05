#pragma once

#include <string>

#include "Core/UUID.h"
#include "Math/Math.h"

namespace Raito::ECS
{
	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const UUID& id) : ID(id){}
		IDComponent(const IDComponent&) = default;
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}
	};

	struct TransformComponent
	{
		v3 Translation = { 0.0f, 0.0f, 0.0f };
		v4 Rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
		v3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const v3& translation)
			: Translation(translation) {}

		matrix GetTransform() const
		{
			return Math::CreateTransform(Translation, Rotation, Scale);
		}
	};

}