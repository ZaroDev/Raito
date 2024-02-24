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



namespace Raito
{
	struct SysWindow;
	namespace Renderer::OpenGL
	{
		enum class FrameBufferTextureFormat
		{
			None = 0,

			// Color
			RGBA8,
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

		class OpenGLFrameBuffer final
		{
		public:
			OpenGLFrameBuffer(const FrameBufferData& spec);
			~OpenGLFrameBuffer();

			DEFAULT_MOVE_AND_COPY(OpenGLFrameBuffer)

			void Bind() const;
			void UnBind() const;
			void Resize(u32 width, u32 height);

			NODISCARD const FrameBufferData& Data() const { return m_Data; }
			NODISCARD u32 ColorAttachment(u32 index = 0) const { ASSERT(index < m_ColorAttachments.size()); return m_ColorAttachments[index]; }
			void Reset();
		private:

			u32 m_RenderId = 0;
			FrameBufferData m_Data;

			std::vector<u32> m_ColorAttachments;
			std::vector<FrameBufferTextureData> m_ColorAttachmentData;

			u32 m_DepthAttachment = 0;
			FrameBufferTextureData m_DepthBufferData = FrameBufferTextureFormat::None;
		};
	}
}
