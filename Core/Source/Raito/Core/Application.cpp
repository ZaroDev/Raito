/*
MIT License

Copyright (c) 2023 Víctor Falcón Zaro

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "pch.h"
#include "Application.h"

#include <utility>


// Modules
#include "Window/Window.h"
#include "Renderer/Renderer.h"
#include "Assets/Assets.h"

#include "Time/ScopedTimer.h"
#include "Time/Time.h"
#include <optick/include/optick.h>

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
		if(!Renderer::SetPlatformInterface(m_Info.GraphicsAPI))
		{
			LOG("Application", "Failed to set graphics API");
			return Failed();
		}
		
		if (!Window::Initialize(m_Info.GraphicsAPI,{ .Title = m_Info.Name,.Height = m_Info.Height, .Width = m_Info.Width, .Fullscreen = m_Info.Fullscreen }))
		{
			LOG("Application", "Failed to initialize window module");
			return Failed();
		}
		if (!Renderer::Initialize())
		{
			LOG("Application", "Failed to initialize renderer module");
			return Failed();
		}

		if(!Assets::Initialize())
		{
			LOG("Application", "Failed to initialize assets module");
			return Failed();
		}

		OnInit();

		return m_Running;
	}
	bool Application::Update()
	{
		while (m_Running)
		{
			OPTICK_FRAME("Main Thread");

			Time::StartTimeUpdate();

			OnUpdate();

			Window::Update();

			OnRenderGUI();

			Time::EndTimeUpdate();
		}
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

		app.m_Info = std::move(info);
		Application::s_Application = &app;

		app.Initialize();

		app.Update();

		app.Shutdown();
		return EXIT_SUCCESS;
	}
}
