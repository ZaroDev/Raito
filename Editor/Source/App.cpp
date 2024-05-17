#include "App.h"
#include "EditorCommon.h"

#include "ImGui/ImGuiEditor.h"

#include "Panels/Panel.h"

#include "Panels/Performance.h"
#include "Panels/Shaders.h"
#include "Panels/Hierarchy.h"

#include <filesystem>

#include "Panels/Assets.h"
#include "Panels/Framebuffers.h"

namespace Editor
{

	bool App::OnInit()
	{
		E_LOG("Application Initialized!");
		ImGuiEditor::Init();

		m_Panels.emplace_back(std::make_unique<Performance>());
		m_Panels.emplace_back(std::make_unique<Shaders>());
		m_Panels.emplace_back(std::make_unique<Hierarchy>());
		m_Panels.emplace_back(std::make_unique<Assets>());
		m_Panels.emplace_back(std::make_unique<Framebuffers>());

		Raito::Assets::ImportModel("Meshes/Sponza/NewSponza_Main_glTF_002.gltf");
		Raito::Assets::ImportModel("Meshes/DamagedHelmet/DamagedHelmet.gltf");
		//Raito::Assets::ImportModel("Meshes/Porsche911/scene.gltf");
		//Raito::Assets::ImportModel("Meshes/WaterBottle/WaterBottle.gltf");
		
		for (u32 i = 0; i < NUM_POINT; i++)
		{
			Raito::V3 position = Raito::Random::Vec3(-10.f, 10.f);
			Raito::V3 color = Raito::Random::Vec3(0.5f, 1.0f);

			Raito::ECS::Entity entity = Scene.CreateEntity("Point Light");
			entity.AddComponent<Raito::ECS::LightComponent>(
				Raito::ECS::LightComponent::Type::POINT_LIGHT,
				color
			);
			entity.AddComponent<Raito::ECS::MeshComponent>(
				Raito::Assets::GetDefaultSphere(),
				Raito::Assets::GetChessMaterial()
			);
			entity.AddOrReplaceComponent<Raito::ECS::TransformComponent>(
				position,
				Raito::Quaternion{ Raito::V3(0.0f) },
				Raito::V3(1.0f)
			);
		}
		{
			Raito::ECS::Entity entity = Scene.CreateEntity("Directional light");
			entity.AddComponent<Raito::ECS::LightComponent>(
				Raito::ECS::LightComponent::Type::DIRECTIONAL,
				Raito::V3{  1.0f },
				Raito::V3{ 20.f, -1.0f, -0.3f }
			);
			entity.AddComponent<Raito::ECS::MeshComponent>(
				Raito::Assets::GetDefaultSphere(),
				Raito::Assets::GetChessMaterial()
			);
			entity.AddOrReplaceComponent<Raito::ECS::TransformComponent>(
				Raito::V3{ 0.0f, 0.0f, 0.0f },
				Raito::Quaternion{ Raito::V3(0.0f) },
				Raito::V3(1.0f)
			);
		}

		return true;
	}

	bool App::OnUpdate()
	{
		for (const auto& p : m_Panels)
		{
			p->Update();
		}

		return m_Running;
	}

	bool App::OnRenderGUI()
	{
		ImGuiEditor::Begin();

		for (const auto& p : m_Panels)
		{
			p->Render();
		}

		ImGuiEditor::End();

		return m_Running;
	}
	void App::OnShutdown()
	{
		ImGuiEditor::Shutdown();
	}
}

CREATE_AND_RUN(Editor::App, Raito::Core::ApplicationInfo("Editor", 1280, 720, Raito::Renderer::API::OPENGL, false))