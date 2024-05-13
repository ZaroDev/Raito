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

#include "Renderer.h"
#include "Shader.h"

namespace Raito::Assets
{
	struct Mesh;
	struct Texture;
}

namespace Raito::Renderer
{
	//! Render hardware interface structure
	//! General abstraction for the rendering interface
	
	struct RHI
	{
		bool(*Initialize)();
		void(*Shutdown)();

		struct
		{
			Surface(*Create)(SysWindow*);
			void(*Remove)(u32);
			void(*Resize)(u32, u32, u32);
			u32(*Width)(u32);
			u32(*Height)(u32);
			void(*Render)(u32);
			u32(*GetColorAttachment)(u32, u32);
			u32(*GetDepthAttachment)(u32);
			u32(*GetDeferredAttachment)(u32);
		} Surface;

		struct
		{
			Shader* (*GetShader)(u32);
			Shader* (*GetShaderId)(EngineShader);
			const std::vector<Shader*>& (*GetAllShaders)();
			ShaderFileData(*GetFileData)(EngineShader);
			u32(*CompileShader)(const ShaderFileData&);
		} Shader;

		struct
		{
			u32(*AddMesh)(Assets::Mesh*);
			void(*RemoveMesh)(u32);

		} Meshes;

		struct
		{
			u32(*AddTexture)(Assets::Texture*, ubyte* data);
			void(*RemoveTexture)(u32);
		} Textures;

		struct
		{
			u32(*AddMaterial)(EngineShader);
			void(*SetMaterialValue)(u32, const char*, ubyte* data, size_t size);
			void(*RemoveMaterial)(u32);
		} Materials;

	};
}
