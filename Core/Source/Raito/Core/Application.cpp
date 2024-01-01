#include "pch.h"
#include "Application.h"


// Modules
#include "Window/Window.h"
#include "Renderer/Renderer.h"

#include "Time/ScopedTimer.h"

namespace Raito::Core
{
	namespace
	{
		bool Failed(const char* msg)
		{
			//LOG("Application", msg);
			Application::Get().Close();
			return false;
		}
	}

	bool Application::Initialize()
	{
		m_Running = true;

		// Init all modules
		if (!Window::Initialize({ .Title = m_Info.Name,.Height = m_Info.Height, .Width = m_Info.Width, .Fullscreen = m_Info.Fullscreen }))
		{
			return Failed("Failed to initialize window module");
		}
		if (!Renderer::Initialize())
		{
			return Failed("Failed to initialize renderer module");
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

Raito::Core::ApplicationInfo CreateInfo(const char* name, u32 width, u32 height)
{
	Raito::Core::ApplicationInfo info{};
	info.Name = name;
	info.Width = width;
	info.Height = height;

	return info;
}
