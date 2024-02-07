#include "pch.h"
#include "Renderer.h"
#include "RHI.h"

#include "D3D12/D3D12RHI.h"
#include "OpenGL/OpenGLRHI.h"

namespace Raito::Renderer
{
	namespace
	{
		RHI g_GraphicsContext{};
		API g_GraphicsAPI = API::NONE;

		bool SetPlatformInterface()
		{
			switch (g_GraphicsAPI)
			{
			case API::NONE: ASSERT(false); return false;
			case API::D3D12: D3D12::GetPlatformInterface(g_GraphicsContext); break;
			case API::OPENGL: OpenGL::GetPlatformInterface(g_GraphicsContext); break;
			case API::COUNT: ASSERT(false); return false;
			}
			return true;
		}
	}


	bool Initialize(API api)
	{
		ASSERT(api != API::NONE && api != API::COUNT);
		LOG("Renderer", "Initializing Renderer");
		g_GraphicsAPI = api;
		return SetPlatformInterface() && g_GraphicsContext.Initialize();
	}
	void Shutdown()
	{
		g_GraphicsContext.Shutdown();
	}

	API GetCurrentAPI()
	{
		return g_GraphicsAPI;
	}

}