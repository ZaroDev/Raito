#include "App.h"
#include "EditorCommon.h"
#include "imgui.h"
#include "ImGui/ImGuiEditor.h"

namespace Editor
{

	bool App::OnInit()
	{
		E_LOG("Application Initialized!");
		ImGuiEditor::Init();
		return true;
	}

	bool App::OnUpdate()
	{
		return m_Running;
	}

	bool App::OnRenderGUI()
	{
		ImGuiEditor::Begin();

		ImGui::ShowDemoWindow();

		ImGuiEditor::End();

		return m_Running;
	}
	void App::OnShutdown()
	{
		ImGuiEditor::Shutdown();
	}
}

CREATE_AND_RUN(Editor::App, Raito::Core::ApplicationInfo( "Editor", 1280, 720, Raito::Renderer::API::OPENGL, false ))