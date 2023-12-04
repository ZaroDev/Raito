#include "pch.h"
#include "Application.h"
#include "Log.h"
#include "Time/ScopedTimer.h"

namespace Raito::Core
{
	bool Application::Initialize()
	{
		m_Running = true;

		// TODO: Init all modules

		return m_Running;
	}
	bool Application::Update()
	{
		ScopedTimer timer("Update");

		// TODO: Module Update

		OnUpdate();
		OnRenderGUI();

		return m_Running;
	}
	void Application::Shutdown()
	{
	}
	int RunApp(Application app, ApplicationInfo info)
	{
		if (Application::s_Application)
		{
			ERR("Application Initalization", "Application was already initialized!");
			return EXIT_FAILURE;
		}

		app.m_Info = info;
		app.s_Application = &app;

		app.Initialize();

		while (app.Update())
		{
			// TODO: Analitics performance dumping
		}

		app.Shutdown();
		return EXIT_SUCCESS;
	}
}

Raito::Core::ApplicationInfo CreateInfo(const char* name, u32 width, u32 height)
{
	Raito::Core::ApplicationInfo info{};
	info.Name = name;
	info.Width = width;
	info.Height = height;

	return info;
}
