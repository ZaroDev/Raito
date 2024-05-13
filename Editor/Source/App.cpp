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

		//Raito::Assets::ImportModel("Meshes/Sponza/NewSponza_Main_glTF_002.gltf");
		Raito::Assets::ImportModel("Meshes/DamagedHelmet/DamagedHelmet.gltf");
		//Raito::Assets::ImportModel("Meshes/Porsche911/scene.gltf");
		//Raito::Assets::ImportModel("Meshes/WaterBottle/WaterBottle.gltf");

		for (u32 i = 0; i < 32; i++)
		{
			const float xPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
			const float yPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 4.0);
			const float zPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);


			const float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
			const float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
			const float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)

			Raito::ECS::Entity entity = Scene.CreateEntity("Light");
			entity.AddComponent<Raito::ECS::LightComponent>(Raito::ECS::LightComponent::Type::POINT_LIGHT ,Raito::V3{ rColor , gColor , bColor });
			entity.AddOrReplaceComponent<Raito::ECS::TransformComponent>(
				Raito::V3{ xPos, yPos, zPos },
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