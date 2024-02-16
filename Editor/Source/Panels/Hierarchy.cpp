#include "Hierarchy.h"

#include <imgui.h>

#include "EditorCommon.h"

namespace Editor
{
	void Hierarchy::Update()
	{

	}

	void Hierarchy::Render()
	{
		ImGui::Begin(m_Name.c_str());

		Raito::ECS::Scene& scene = Raito::Core::Application::Get().Scene;

		auto view = scene.GetAllEntitiesWith<Raito::ECS::TagComponent, Raito::ECS::TransformComponent, Raito::ECS::MeshComponent>();

		for (auto& ent : view)
		{
			const auto& tag = view.get<Raito::ECS::TagComponent>(ent);
			const auto& transform = view.get<Raito::ECS::TransformComponent>(ent);
			const auto& mesh = view.get<Raito::ECS::MeshComponent>(ent);


			ImGui::Separator();
			ImGui::Text(tag.Tag.c_str());
			ImGui::Text("Mesh id: %i", mesh.MeshId);
			ImGui::Text("Transform");
			ImGui::Text("Pos: %.2f %.2f %.2f", transform.Translation.x, transform.Translation.y, transform.Translation.z);
			ImGui::Text("Rot: %.2f %.2f %.2f %.2f", transform.Rotation.x, transform.Rotation.y, transform.Rotation.z, transform.Rotation.w);
			ImGui::Text("Scl: %.2f %.2f %.2f", transform.Scale.x, transform.Scale.y, transform.Scale.z);
		}


		ImGui::End();
	}
}