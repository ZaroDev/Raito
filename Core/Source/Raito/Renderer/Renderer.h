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
			NODISCARD u32 SSAOAttachment()const;
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

		//! Sets the platform interface
		//! @param api The desired api to be used
		//! @return If the platform is implemented
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

		//! Create surface for window
		//! @param window The window to create the surface
		//! @return The created surface
		NODISCARD Surface CreateSurface(SysWindow* window);

		//! Remove a surface
		//! @param id The id of the surface
		void RemoveSurface(u32 id);

		//! Get shader file data
		//! @param id Shader ID
		//! @return The shader file data
		NODISCARD ShaderFileData GetFileData(EngineShader id);

		//! Get all the compiled shaders
		//! @return A reference list of the compiled shaders
		NODISCARD const std::vector<Shader*>& GetAllShaders();

		//! Compiles the shader provided
		//! @param data Contains the information of the shader file
		//! @return The id of the compiled shader
		NODISCARD u32 CompileShader(const ShaderFileData& data);

		//! Creates a mesh and uploads it to the GPU
		//! @param mesh CPU mesh to be uploaded
		//! @return The id of the GPU mesh data
		u32 AddMesh(Assets::Mesh* mesh);

		//! Removes the given mesh from the GPU
		//! @param id of the GPU mesh
		void RemoveMesh(u32 id);

		//! Creates a texture on the GPU and uploads the data
		//! @param texture A CPU texture containing the metadata of the texture
		//! @param data The actual bytes that compose the texture
		//! @return The id of the generated texture
		u32 AddTexture(Assets::Texture* texture, ubyte* data);

		//! Removes a texture from the GPU
		//! @param id of the texture on the CPU
		void RemoveTexture(u32 id);

		//! Creates a CPU material to then uploaded when applied to a mesh
		//! @param shader The base shader that the material will apply the data to
		//! @return The id of the material on the renderer
		u32 AddMaterial(EngineShader shader);

		//! Sets a material value using raw data
		//! Similar to memcpy()
		//! @param id of the material
		//! @param name of the value to change
		//! @param data to copy to
		//! @param size of the data in bytes
		void SetMaterialValue(u32 id, const char* name, ubyte* data, size_t size);

		//! Removes a material from the renderer
		//! @param id of the material
		void RemoveMaterial(u32 id);

		//! Get the location of compiled engine shaders relative to the executable
		//! The path is for the graphics API that's currently in use.
		//! @param API The desired graphics API path
		//! @return The folder name
		NODISCARD const char* GetEngineShadersPath(API API);
	}
}
