#include "Hierarchy.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

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
			auto& transform = view.get<Raito::ECS::TransformComponent>(ent);
			const auto& mesh = view.get<Raito::ECS::MeshComponent>(ent);


			ImGui::Separator();
			ImGui::Text(tag.Tag.c_str());
			ImGui::Text("Mesh id: %i", mesh.MeshId);
			ImGui::Text("Transform");

			
			ImGui::DragFloat3("Pos", (float*)&transform.Translation);

			Raito::V3 rot = glm::degrees(glm::eulerAngles(transform.Rotation));
			if(ImGui::DragFloat3("Rot", (float*)&rot))
			{
				transform.Rotation = Raito::Quaternion(rot);
			}
			ImGui::DragFloat3("Scl", (float*)&transform.Scale);
		}


		ImGui::End();
	}
}