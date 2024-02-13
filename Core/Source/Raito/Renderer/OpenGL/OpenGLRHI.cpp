#include <pch.h>
#include "OpenGLRHI.h"

#include "Renderer/RHI.h"
#include "OpenGLCore.h"
#include "OpenGLShaderCompiler.h"

namespace Raito::Renderer::OpenGL
{
	void GetPlatformInterface(RHI& rhi)
	{
		rhi.Initialize = Initialize;
		rhi.Shutdown = Shutdown;

		rhi.Surface.Create = CreateSurface;
		rhi.Surface.Remove = RemoveSurface;

		rhi.Surface.Resize = ResizeSurface;
		rhi.Surface.Render = RenderSurface;
		rhi.Surface.Height = SurfaceHeight;
		rhi.Surface.Width = SurfaceWidth;

		rhi.Shader.GetShader = ShaderCompiler::GetShader;
		rhi.Shader.GetShaderId = ShaderCompiler::GetShaderWithEngineId;
		rhi.Shader.GetFileData = ShaderCompiler::GetFileData;
		rhi.Shader.GetAllShaders = ShaderCompiler::GetAllShaders;
		rhi.Shader.CompileShader = ShaderCompiler::CompileShader;

		rhi.Meshes.AddMesh = AddMesh;
		rhi.Meshes.RemoveMesh = RemoveMesh;
	}
}
