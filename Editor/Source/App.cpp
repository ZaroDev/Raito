#include "App.h"
#include "EditorCommon.h"
#include "imgui.h"
#include "ImGui/ImGuiEditor.h"
#include "Panels/Panel.h"
#include "Panels/Performance.h"
#include "Panels/Shaders.h"

#include <filesystem>

namespace Editor
{

	bool App::OnInit()
	{
		E_LOG("Application Initialized!");
		ImGuiEditor::Init();

		m_Panels.emplace_back(std::make_unique<Performance>());
		m_Panels.emplace_back(std::make_unique<Shaders>());

		Raito::Assets::ImportModel("Meshes/NewSponza_Main_glTF_002.gltf");

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

CREATE_AND_RUN(Editor::App, Raito::Core::ApplicationInfo( "Editor", 1280, 720, Raito::Renderer::API::OPENGL, false ))