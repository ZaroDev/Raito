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

		{
			const auto view = scene.GetAllEntitiesWith<Raito::ECS::TagComponent, Raito::ECS::TransformComponent, Raito::ECS::MeshComponent>();

			for (auto& ent : view)
			{
				const auto& tag = view.get<Raito::ECS::TagComponent>(ent);
				auto& transform = view.get<Raito::ECS::TransformComponent>(ent);
				const auto& mesh = view.get<Raito::ECS::MeshComponent>(ent);
				ImGui::PushID(tag.Tag.c_str());

				ImGui::Separator();
				ImGui::Text(tag.Tag.c_str());
				ImGui::Text("Mesh id: %i", mesh.MeshId);
				ImGui::Text("Transform");


				ImGui::DragFloat3("Pos", (float*)&transform.Translation);

				Raito::V3 rot = glm::degrees(glm::eulerAngles(transform.Rotation));
				if (ImGui::DragFloat3("Rot", (float*)&rot))
				{
					transform.Rotation = Raito::Quaternion(glm::radians(rot));
				}
				ImGui::DragFloat3("Scl", (float*)&transform.Scale);

				ImGui::PopID();
			}
		}
		{
			const auto view = scene.GetAllEntitiesWith<Raito::ECS::TagComponent, Raito::ECS::TransformComponent, Raito::ECS::LightComponent>();

			for (auto& ent : view)
			{
				const auto& tag = view.get<Raito::ECS::TagComponent>(ent);
				auto& transform = view.get<Raito::ECS::TransformComponent>(ent);
				const auto& light = view.get<Raito::ECS::LightComponent>(ent);

				ImGui::PushID(tag.Tag.c_str());

				ImGui::Separator();
				ImGui::Text(tag.Tag.c_str());
				ImGui::Text("Transform");
				ImGui::DragFloat3("Pos", (float*)&transform.Translation);

				Raito::V3 rot = glm::degrees(glm::eulerAngles(transform.Rotation));
				if (ImGui::DragFloat3("Rot", (float*)&rot))
				{
					transform.Rotation = Raito::Quaternion(glm::radians(rot));
				}
				ImGui::DragFloat3("Scl", (float*)&transform.Scale);

				ImGui::ColorEdit3("Light Color", (float*)&light.Color);

				ImGui::PopID();
			}
		}



		ImGui::End();
	}
}