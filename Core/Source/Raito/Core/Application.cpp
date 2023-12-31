#include "pch.h"
#include "Application.h"


// Modules
#include "Window/Window.h"
#include "Renderer/Renderer.h"

#include "Time/ScopedTimer.h"

namespace Raito::Core
{
	
	bool Failed()
	{
		Application::Get().Close();
		return false;
	}

	bool Application::Initialize()
	{
		m_Running = true;

		// Init all modules
		if (!Debug::Initialize(true))
		{
			return Failed();
		}

		if (!Window::Initialize({ .Title = m_Info.Name,.Height = m_Info.Height, .Width = m_Info.Width, .Fullscreen = m_Info.Fullscreen }))
		{
			LOG("Application", "Failed to initialize window module");
			return Failed();
		}
		if (!Renderer::Initialize())
		{
			LOG("Application", "Failed to initialize renderer module");
			return Failed();
		}

		OnInit();

		return m_Running;
	}
	bool Application::Update()
	{
		//ScopedTimer timer("Update");

		// TODO: Module Update

		Window::Update();

		return m_Running;
	}
	void Application::Shutdown()
	{
		Window::Shutdown();

		Renderer::Shutdown();


		OnShutdown();
	}

	int RunApp(Application& app, ApplicationInfo info, int argc, char** argv)
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
