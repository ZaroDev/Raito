#include "ImGuiBackend.h"
#include <Raito/Raito.h>

#include "OpenGLImGuiBackend.h"
#include "Raito/Renderer/Renderer.h"

namespace Editor::ImGuiBackend
{
	namespace 
	{
		ImGuiBackendRHI g_BackendInterface;
		void GetImGuiInterface()
		{
			switch (Raito::Renderer::GetCurrentAPI())
			{
			case Raito::Renderer::API::D3D12:
				break;
			case Raito::Renderer::API::OPENGL: OpenGL::GetOpenGLBackend(g_BackendInterface);
				break;
			}
		}
	}

	void Init()
	{
		GetImGuiInterface();
		g_BackendInterface.Init();
	}

	void NewFrame()
	{
		g_BackendInterface.NewFrame();
	}

	void Render()
	{
		g_BackendInterface.Render();
	}

	void Shutdown()
	{
		g_BackendInterface.Shutdown();
	}
}
