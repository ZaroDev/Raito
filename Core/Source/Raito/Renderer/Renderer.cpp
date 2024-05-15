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

	u32 Surface::ColorAttachment(u32 id) const
	{
		return g_GraphicsContext.Surface.GetColorAttachment(m_Id, id);
	}

	u32 Surface::DeferredAttachment(u32 id) const
	{
		return g_GraphicsContext.Surface.GetDeferredAttachment(id);
	}

	u32 Surface::DeferredDepth() const
	{
		return g_GraphicsContext.Surface.GetDeferredDepth();
	}

	u32 Surface::DepthAttachment() const
	{
		return g_GraphicsContext.Surface.GetDepthAttachment(m_Id);
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

	u32 AddTexture(Assets::Texture* texture, ubyte* data)
	{
		return g_GraphicsContext.Textures.AddTexture(texture, data);
	}

	void RemoveTexture(u32 id)
	{
		g_GraphicsContext.Textures.RemoveTexture(id);
	}

	u32 AddMaterial(EngineShader shader)
	{
		return g_GraphicsContext.Materials.AddMaterial(shader);
	}

	void SetMaterialValue(u32 id, const char* name, ubyte* data, size_t size)
	{
		g_GraphicsContext.Materials.SetMaterialValue(id, name, data, size);
	}

	void RemoveMaterial(u32 id)
	{
		g_GraphicsContext.Materials.RemoveMaterial(id);
	}
}
