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
#include <Raito/Renderer/OpenGL/OpenGLCommon.h>

namespace Raito
{
	struct SysWindow;
	namespace Renderer::OpenGL
	{
		class OpenGLFrameBuffer final
		{
		public:
			DEFAULT_MOVE_AND_COPY(OpenGLFrameBuffer)

			OpenGLFrameBuffer() = delete;
			explicit OpenGLFrameBuffer(const FrameBufferData& spec);
			~OpenGLFrameBuffer();


			void Bind() const;
			void UnBind() const;
			void Resize(u32 width, u32 height);
			void Reset();

			NODISCARD u32 Id() const { return m_RenderId; }
			NODISCARD const FrameBufferData& Data() const { return m_Data; }
			NODISCARD u32 ColorAttachment(u32 index = 0) const { ASSERT(index < m_ColorAttachments.size()); return m_ColorAttachments[index]; }
			NODISCARD u32 DepthAttachment() const { return m_DepthAttachment; }
		protected:

			u32 m_RenderId = 0;
			FrameBufferData m_Data;

			std::vector<u32> m_ColorAttachments;
			std::vector<FrameBufferTextureData> m_ColorAttachmentData;

			u32 m_DepthAttachment = 0;
			FrameBufferTextureData m_DepthBufferData = FrameBufferTextureFormat::None;
		};
	}
}
