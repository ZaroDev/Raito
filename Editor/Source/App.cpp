#include "App.h"
#include "EditorCommon.h"

namespace Editor
{

	bool App::OnInit()
	{
		E_LOG("Application Initialized!");
		
		return true;
	}

	bool App::OnUpdate()
	{
		return m_Running;
	}

	bool App::OnRenderGUI()
	{
		return m_Running;
	}
	void App::OnShutdown()
	{
	}
}

CREATE_AND_RUN(Editor::App, Raito::Core::ApplicationInfo( "Editor", 1280, 720, Raito::Renderer::API::OPENGL, false ))