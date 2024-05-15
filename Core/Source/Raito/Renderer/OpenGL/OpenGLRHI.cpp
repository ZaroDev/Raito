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
		rhi.Surface.GetColorAttachment = GetColorAttachment;
		rhi.Surface.GetDepthAttachment = GetDepthAttachment;
		rhi.Surface.GetDeferredAttachment = GetDeferredBufferAttachment;
		rhi.Surface.GetDeferredDepth = GetDeferredBufferDepth;
		rhi.Surface.Width = SurfaceWidth;

		rhi.Shader.GetShader = ShaderCompiler::GetShader;
		rhi.Shader.GetShaderId = ShaderCompiler::GetShaderWithEngineId;
		rhi.Shader.GetFileData = ShaderCompiler::GetFileData;
		rhi.Shader.GetAllShaders = ShaderCompiler::GetAllShaders;
		rhi.Shader.CompileShader = ShaderCompiler::CompileShader;

		rhi.Meshes.AddMesh = AddMesh;
		rhi.Meshes.RemoveMesh = RemoveMesh;

		rhi.Textures.AddTexture = AddTexture;
		rhi.Textures.RemoveTexture = RemoveTexture;

		rhi.Materials.AddMaterial = AddMaterial;
		rhi.Materials.SetMaterialValue = SetMaterialValue;
		rhi.Materials.RemoveMaterial = RemoveMaterial;
	}
}
