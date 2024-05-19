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
#pragma once

#include "OpenGLCommon.h"
#include "OpenGLObjects/OpenGLMaterial.h"

namespace Raito::Renderer::OpenGL
{
	bool Initialize();
	void Shutdown();


    NODISCARD Surface CreateSurface(SysWindow* window);
    void RemoveSurface(u32 id);
    void ResizeSurface(u32 id, u32 width, u32 height);
    NODISCARD u32 SurfaceWidth(u32 id);
    NODISCARD u32 SurfaceHeight(u32 id);

    NODISCARD u32 GetColorAttachment(u32 target, u32 id);
    NODISCARD u32 GetDepthAttachment(u32 id);
    NODISCARD u32 GetDeferredBufferAttachment(u32 id);
    NODISCARD u32 GetDeferredBufferDepth();

    void RenderSurface(u32 id);

    u32 AddMesh(Assets::Mesh* mesh);
    const OpenGLMeshData& GetMesh(u32 id);
    void RemoveMesh(u32 id);

    u32 AddTexture(Assets::Texture* texture, ubyte* data);
    void RemoveTexture(u32 id);

    u32 AddMaterial(EngineShader shader);
    void SetMaterialValue(u32 id, const char* name, ubyte* data, size_t size);
	OpenGLMaterial& GetMaterial(u32 id);
    void RemoveMaterial(u32 id);
}
