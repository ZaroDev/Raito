#include "App.h"

namespace Editor
{

	bool App::OnInit()
	{
		Raito::ECS::Entity entity = m_Scene.CreateEntity("Test entity");
		
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

CREATE_AND_RUN(Editor::App, Raito::Core::ApplicationInfo( "Editor", 1280, 720, false ))