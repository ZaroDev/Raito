#include "App.h"

#include <iostream>
namespace Editor
{
	App::App()
	{
	}

	bool App::OnInit()
	{
		Raito::ECS::Entity entt = m_Scene.CreateEntity("Test entity");
		std::cout << "Created entt" << std::endl;
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

CREATE_AND_RUN(Editor::App, CreateInfo("Editor"))