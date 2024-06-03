#include <pch.h>
#include "OpenGLPostProcessPass.h"

#include "optick/include/optick.h"
#include "Renderer/OpenGL/OpenGLCore.h"
#include "Renderer/OpenGL/OpenGLObjects/OpenGLShader.h"
#include "Window/Window.h"

namespace Raito::Renderer::OpenGL::PostProcess
{
	namespace
	{
	
		bool g_EnableBloom = true;

		constexpr u32 c_BloomMipLevels = 3;
		u32 g_BloomFBO;
		constexpr float c_BloomFilterRadius = 0.005f;

		u32 g_BloomTextureUniform;
		u32 g_ColorTextureUniform;


		struct BloomMip
		{
			V2 Size;
			Iv2 IntSize;
			u64 Handle;
			GLuint Texture;
		};
		std::vector<BloomMip> g_BloomMipChains{};

		GLint g_EnableBloomUniform;

		bool InitBloomBuffer()
		{
			const SysWindow& window = Raito::Window::GetWindow(0);


			glGenFramebuffers(1, &g_BloomFBO);
			glBindFramebuffer(GL_FRAMEBUFFER, g_BloomFBO);

			V2 mipSize = { window.Info.Width, window.Info.Height };
			Iv2 mipIntSize = { window.Info.Width, window.Info.Height };

			for (u32 i = 0; i < c_BloomMipLevels; i++)
			{
				BloomMip mip;
				mipSize *= 0.5f;
				mipIntSize /= 2;

				mip.Size = mipSize;
				mip.IntSize = mipIntSize;

				glGenTextures(1, &mip.Texture);
				glBindTexture(GL_TEXTURE_2D, mip.Texture);

			

				glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, mipIntSize.x, mipIntSize.y, 0, GL_RGB, GL_FLOAT, nullptr);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				mip.Handle = glGetTextureHandleARB(mip.Texture);
				glMakeTextureHandleResidentARB(mip.Handle);
				g_BloomMipChains.emplace_back(mip);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_BloomMipChains[0].Texture, 0);
			constexpr u32 attachments[1] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, attachments);

			const i32 status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE)
			{
				O_ERROR("Gbuffer FBO error, status: 0x{0}", status);

				return false;
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			//Setup shader texture layout values
			{
				const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(BLOOM_DOWN_SAMPLE));
				shader->Bind();
				shader->SetUniform("u_ScreenTexture", 0);
				shader->UnBind();
			}
			{
				const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(BLOOM_UP_SAMPLE));
				shader->Bind();
				shader->SetUniform("u_ScreenTexture", 0);
				shader->UnBind();
			}
			{
				const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(POST_PROCESS));
				shader->Bind();
				g_BloomTextureUniform = shader->GetUniformLocation("u_BloomTexture");
				g_ColorTextureUniform = shader->GetUniformLocation("u_ScreenTexture");
				g_EnableBloomUniform = shader->GetUniformLocation("u_EnableBloom");
				shader->UnBind();
			}

			return true;
		}
	}


	bool Initialize()
	{
		
		InitBloomBuffer();
		return true;
	}

	void EnableBloom(bool value)
	{
		g_EnableBloom = value;
	}

	void Update(const OpenGLFrameBuffer& buffer)
	{
		OPTICK_CATEGORY("Update post processing", Optick::Category::Rendering);


		// Bloom
		if(g_EnableBloom)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, g_BloomFBO);

			// Bloom down-sample
			{
				const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(BLOOM_DOWN_SAMPLE));
				shader->Bind();
				shader->SetUniformRef("u_Resolution", V2{ buffer.Data().Width, buffer.Data().Height });

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, buffer.ColorAttachment(1));

				for (const auto& mip : g_BloomMipChains)
				{
					glViewport(0, 0, mip.IntSize.x, mip.IntSize.y);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.Texture, 0);

					RenderFullScreenQuad();
					
					shader->SetUniformRef("u_Resolution", mip.Size);
					glBindTexture(GL_TEXTURE_2D, mip.Texture);
				}

				shader->UnBind();
			}

			// Bloom up-sample
			{
				const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(BLOOM_UP_SAMPLE));
				shader->Bind();
				shader->SetUniform("u_FilterRadius", c_BloomFilterRadius);

				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);
				glBlendEquation(GL_FUNC_ADD);

				for (u32 i = g_BloomMipChains.size() - 1; i > 0; i--)
				{
					const BloomMip& mip = g_BloomMipChains[i];
					const BloomMip& nextMip = g_BloomMipChains[i - 1];

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, mip.Texture);

					glViewport(0, 0, nextMip.IntSize.x, nextMip.IntSize.y);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						GL_TEXTURE_2D, nextMip.Texture, 0);

					// Render screen-filled quad of resolution of current mip
					RenderFullScreenQuad();
				}
				glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // Restore if this was default
				glDisable(GL_BLEND);
				shader->UnBind();
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, buffer.Data().Width, buffer.Data().Height);
		}

		// Render to quad
		{
			glClear(GL_COLOR_BUFFER_BIT);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(POST_PROCESS));
			shader->Bind();

			
			glDisable(GL_DEPTH_TEST);

			glUniformHandleui64ARB(g_ColorTextureUniform, buffer.ColorHandle());
			glUniformHandleui64ARB(g_BloomTextureUniform, g_BloomMipChains[0].Handle);

			shader->SetUniform(g_EnableBloomUniform, static_cast<i32>(g_EnableBloom));

			RenderFullScreenQuad();

			shader->UnBind();
		}
	}

	void Shutdown()
	{

		for (int i = 0; i < g_BloomMipChains.size(); i++)
		{
			glDeleteTextures(1, &g_BloomMipChains[i].Texture);
			g_BloomMipChains[i].Texture = 0;
		}
		glDeleteFramebuffers(1, &g_BloomFBO);
		g_BloomFBO = 0;
	}
}
