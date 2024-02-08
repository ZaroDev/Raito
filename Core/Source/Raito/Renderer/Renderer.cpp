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
	}


	void Surface::Resize(u32 width, u32 height) const
	{
		g_GraphicsContext.Surface.Resize(m_Id, width, height);
	}

	u32 Surface::Width() const
	{
		return g_GraphicsContext.Surface.Width(m_Id);
	}

	u32 Surface::Height() const
	{
		return g_GraphicsContext.Surface.Width(m_Id);
	}

	void Surface::Render() const
	{
		g_GraphicsContext.Surface.Render(m_Id);
	}

	bool SetPlatformInterface(API api)
	{
		ASSERT(api != API::NONE && api != API::COUNT);
		g_GraphicsAPI = api;
		switch (g_GraphicsAPI)
		{
		case API::D3D12: D3D12::GetPlatformInterface(g_GraphicsContext); break;
		case API::OPENGL: OpenGL::GetPlatformInterface(g_GraphicsContext); break;
		}
		return true;
	}

	bool Initialize()
	{
		LOG("Renderer", "Initializing Renderer");
		return g_GraphicsContext.Initialize();
	}
	void Shutdown()
	{
		g_GraphicsContext.Shutdown();
	}

	API GetCurrentAPI()
	{
		return g_GraphicsAPI;
	}

	Surface CreateSurface(SysWindow* window)
	{
		return g_GraphicsContext.Surface.Create(window);
	}

	void RemoveSurface(u32 id)
	{
		g_GraphicsContext.Surface.Remove(id);
	}
}
