/*
MIT License

Copyright (c) 2023 V�ctor Falc�n Zaro

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
#include "OpenGLFrameBuffer.h"

#include "glad/glad.h"


namespace Raito::Renderer::OpenGL
{

	namespace
	{
		GLenum TextureTarget(bool multiSampled)
		{
			return multiSampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		void CreateTextures(bool multiSample, u32* data, u32 count)
		{
			glCreateTextures(TextureTarget(multiSample), count, data);
		}

		void BindTexture(bool multiSample, u32 id)
		{
			glBindTexture(TextureTarget(multiSample), id);
		}

		bool IsDepthFormat(FrameBufferTextureFormat format)
		{
			switch (format)
			{
			case FrameBufferTextureFormat::DEPTH24STENCIL8:  return true;
			case FrameBufferTextureFormat::DEPTH32F: return true;
			}

			return false;
		}
		void AttachColorTexture(u32 id, int samples, GLenum internalFormat, GLenum format, u32 width, u32 height, int index)
		{
			const bool multiSampled = samples > 1;
			if (multiSampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multiSampled), id, 0);
		}

		void AttachDepthTexture(u32 id, int samples, GLenum format, GLenum attachmentType, u32 width, u32 height)
		{
			const bool multiSampled = samples > 1;
			if (multiSampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else
			{
				glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multiSampled), id, 0);
		}
	}

	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferData& spec) : m_Data(spec)
	{
		for(auto data : m_Data.Attachments.Attachments)
		{
			if(!IsDepthFormat(data.TextureFormat))
			{
				m_ColorAttachmentData.push_back(data);
			}
			else
			{
				m_DepthBufferData = data;
			}
		}

		Reset();
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		if(m_RenderId)
		{
			glDeleteFramebuffers(1, &m_RenderId);
			glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
			glDeleteTextures(1, &m_DepthAttachment);
		}

		m_RenderId = 0;
		m_ColorAttachments.clear();
		m_ColorAttachmentData.clear();
	}

	void OpenGLFrameBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RenderId);
		glViewport(0, 0, m_Data.Width, m_Data.Height);
	}

	void OpenGLFrameBuffer::UnBind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::Resize(u32 width, u32 height)
	{
		m_Data.Width = width;
		m_Data.Height = height;

		Reset();
	}

	void OpenGLFrameBuffer::Reset()
	{
		if(m_RenderId)
		{
			glDeleteFramebuffers(1, &m_RenderId);
			glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
			glDeleteTextures(1, &m_DepthAttachment);

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}

		glGenFramebuffers(1, &m_RenderId);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RenderId);

		const bool multiSample = m_Data.Samples > 1;

		if(!m_ColorAttachmentData.empty())
		{
			m_ColorAttachments.resize(m_ColorAttachmentData.size());
			m_ColorHandles.resize(m_ColorAttachmentData.size());
			CreateTextures(multiSample, m_ColorAttachments.data(), m_ColorAttachments.size());

			for(u32 i = 0; i < m_ColorAttachments.size(); i++)
			{
				BindTexture(multiSample, m_ColorAttachments[i]);

				switch (m_ColorAttachmentData[i].TextureFormat)
				{
				case FrameBufferTextureFormat::RGBA8:
					AttachColorTexture(m_ColorAttachments[i], m_Data.Samples, GL_RGBA8, GL_RGBA, m_Data.Width, m_Data.Height, i);
					break;
				case FrameBufferTextureFormat::RGBA16F:
					AttachColorTexture(m_ColorAttachments[i], m_Data.Samples, GL_RGBA16F, GL_RGBA, m_Data.Width, m_Data.Height, i);
					break;
				case FrameBufferTextureFormat::RGBA:
					AttachColorTexture(m_ColorAttachments[i], m_Data.Samples, GL_RGBA, GL_RGBA, m_Data.Width, m_Data.Height, i);
					break;
				case FrameBufferTextureFormat::RED_INTEGER:
					AttachColorTexture(m_ColorAttachments[i], m_Data.Samples, GL_R32I, GL_RED_INTEGER, m_Data.Width, m_Data.Height, i);
					break;
				}
				m_ColorHandles[i] = glGetTextureHandleARB(m_ColorAttachments[i]);
				glMakeTextureHandleResidentARB(m_ColorHandles[i]);
			}
		}

		if (m_DepthBufferData.TextureFormat != FrameBufferTextureFormat::None)
		{
			CreateTextures(multiSample, &m_DepthAttachment, 1);
			BindTexture(multiSample, m_DepthAttachment);
			switch (m_DepthBufferData.TextureFormat)
			{
			case FrameBufferTextureFormat::DEPTH24STENCIL8:
				AttachDepthTexture(m_DepthAttachment, m_Data.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Data.Width, m_Data.Height);
				break;
			case FrameBufferTextureFormat::DEPTH32F:
				AttachDepthTexture(m_DepthAttachment, m_Data.Samples, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, m_Data.Width, m_Data.Height);
				break;
			default:
				break;
			}
		}

		if (m_ColorAttachments.size() > 1)
		{
			ASSERT(m_ColorAttachments.size() <= 5);
			constexpr GLenum buffers[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
			glDrawBuffers(m_ColorAttachments.size(), buffers);
		}
		else if (m_ColorAttachments.empty())
		{
			// Only depth-pass
			glDrawBuffer(GL_NONE);
		}

		ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}
}
