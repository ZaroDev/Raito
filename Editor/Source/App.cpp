#include "Raito/Core/Application.h"
#include "App.h"

#include <Windows.h>

namespace Editor
{
	App::App()
	{
	}

	bool App::OnUpdate()
	{
		return m_Running;
	}

	bool App::OnRenderGUI()
	{
		return m_Running;
	}
}
CREATE_AND_RUN(Editor::App, CreateInfo("Editor"))