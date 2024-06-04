#pragma once

#include <imgui.h>
#include <ImGuizmo.h>

#include "Panel.h"
#include "Raito/ECS/Entity.h"

namespace Editor
{
	class Hierarchy final : public Panel
	{
	public:
		Hierarchy() : Panel("Hierarchy")
		{
		}

		virtual ~Hierarchy() = default;

		void Update() override;
		void Render() override;

	private:

		void DrawGizmos();
		void DrawEntityNode(Raito::ECS::Entity entity);
		void DrawComponents(Raito::ECS::Entity entity);
		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);

		Raito::ECS::Scene* m_Context;
		Raito::ECS::Entity m_SelectionContext;
		ImGuizmo::OPERATION m_GizmoType = ImGuizmo::BOUNDS;
		ImGuizmo::MODE m_GizmoMode = ImGuizmo::LOCAL;
	};
}
