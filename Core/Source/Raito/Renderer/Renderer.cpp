#include "pch.h"
#include "Renderer.h"
#include "Shader.h"
#include "RHI.h"

#include "D3D12/D3D12RHI.h"
#include "OpenGL/OpenGLRHI.h"


namespace Raito::Renderer
{

	namespace
	{
		API g_GraphicsAPI = API::NONE;
		RHI g_GraphicsContext{};
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

	ShaderFileData GetFileData(EngineShader id)
	{
		return g_GraphicsContext.Shader.GetFileData(id);
	}

	const std::vector<Shader*>& GetAllShaders()
	{
		return g_GraphicsContext.Shader.GetAllShaders();
	}

	u32 CompileShader(const ShaderFileData& data)
	{
		return g_GraphicsContext.Shader.CompileShader(data);
	}

	u32 AddMesh(Assets::Mesh* mesh)
	{
		return g_GraphicsContext.Meshes.AddMesh(mesh);
	}

	void RemoveMesh(u32 id)
	{
		g_GraphicsContext.Meshes.RemoveMesh(id);
	}
}
