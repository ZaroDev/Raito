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

#include "GraphicsAPI.h"
#include "Shader.h"

namespace Raito
{
	namespace Assets
	{
		struct Mesh;
		struct Texture;
	}

	struct SysWindow;

	namespace Renderer
	{
		class Surface
		{
		public:
			constexpr explicit Surface(u32 id) : m_Id(id) {}
			Surface() = default;

			NODISCARD constexpr u32 Id() const { return m_Id; }

			void Resize(u32 width, u32 height) const;
			NODISCARD u32 Width() const;
			NODISCARD u32 Height() const;
			NODISCARD u32 ColorAttachment(u32 id = 0) const;
			NODISCARD u32 DeferredAttachment(u32 id = 0) const;
			NODISCARD u32 DeferredLightAttachment(u32 id = 0) const;
			NODISCARD u32 DeferredDepth() const;
			NODISCARD u32 DepthAttachment() const;
			void Render() const;
		private:
			u32 m_Id;
		};

		struct RenderSurface
		{
			SysWindow* Window;
			Surface Surface;
		};

		enum class FrameBufferTextureFormat
		{
			None = 0,

			// Color
			RGBA8,
			RGBA16F,
			RGBA,
			RED_INTEGER,

			// Depth/stencil
			DEPTH24STENCIL8,

			// Defaults
			Depth = DEPTH24STENCIL8
		};

		struct FrameBufferTextureData
		{
			FrameBufferTextureData() = default;
			FrameBufferTextureData(FrameBufferTextureFormat format)
				: TextureFormat(format) {}

			FrameBufferTextureFormat TextureFormat = FrameBufferTextureFormat::None;
		};

		struct FrameBufferAttachmentData
		{
			FrameBufferAttachmentData() = default;
			FrameBufferAttachmentData(std::initializer_list<FrameBufferTextureData> attachments)
				: Attachments(attachments) {}

			std::vector<FrameBufferTextureData> Attachments;
		};

		struct FrameBufferData
		{
			FrameBufferAttachmentData Attachments;
			u32 Width, Height;
			u32 Samples = 1;
			bool SwapChainTarget = false;
		};

		enum class LightTechnique
		{
			Forward = 0,
			Deferred,
		};

		//
		//! @param api The desired api to be used 
		bool SetPlatformInterface(API api);

		//! Renderer initialization function
		//! @returns the success
		bool Initialize();

		//! Renderer shutdown function
		//! Releases all renderer objects
		void Shutdown();

		//! Renderer graphics API getter
		//! @return Current graphics API
		NODISCARD API GetCurrentAPI();

		NODISCARD Surface CreateSurface(SysWindow* window);
		void RemoveSurface(u32 id);

		NODISCARD ShaderFileData GetFileData(EngineShader id);
		const std::vector<Shader*>& GetAllShaders();
		u32 CompileShader(const ShaderFileData& data);

		u32 AddMesh(Assets::Mesh* mesh);
		void RemoveMesh(u32 id);

		u32 AddTexture(Assets::Texture* texture, ubyte* data);
		void RemoveTexture(u32 id);

		u32 AddMaterial(EngineShader shader);
		void SetMaterialValue(u32 id, const char* name, ubyte* data, size_t size);
		void RemoveMaterial(u32 id);
	}
}
