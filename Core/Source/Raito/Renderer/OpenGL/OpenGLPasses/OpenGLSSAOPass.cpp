#include <pch.h>
#include "OpenGLSSAOPass.h"

#include "Random/Random.h"
#include "Renderer/OpenGL/OpenGLObjects/OpenGLShader.h"
#include <Raito/Math/Math.h>

#include "Renderer/Camera.h"
#include "Renderer/OpenGL/OpenGLCore.h"

namespace Raito::Renderer::OpenGL::SSAO
{
	namespace
	{
		OpenGLFrameBuffer* g_FrameBuffer;
		OpenGLFrameBuffer* g_BlurBuffer;

		u32 g_NoiseTexture;
		u64 g_NoiseHandle;

		constexpr i32 c_KernelSize = 64;
		std::vector<V3> g_SSAOKernel;

		constexpr u32 c_NoiseSize = 16;
		std::vector<V3> g_SSAONoise;


		struct SSAOUniforms
		{
			GLint Projection;
			GLint Position;
			GLint Normal;
			GLint Noise;
			GLint Size;
			GLint View;
		} g_SSAOUniforms;

		GLint g_SSAOTextureUniform;
		GLint g_SamplesUniforms[c_KernelSize];

		void FillKernel()
		{
			g_SSAOKernel.resize(c_KernelSize);
			for(u32 i = 0; i < c_KernelSize; i++)
			{
				V3 sample = V3
				{
					Random::Float(0.0, 1.0) * 2.0 - 1.0,
					Random::Float(0.0, 1.0) * 2.0 - 1.0,
					Random::Float(0.0, 1.0)
				};

				sample *= glm::normalize(sample);
				sample *= Random::Float(0.0, 1.0);

				float scale = static_cast<float>(i) / static_cast<float>(c_KernelSize);

				scale = Math::Lerp(0.1f, 1.0f, scale * scale);
				sample *= scale;
				g_SSAOKernel[i] = sample;
			}
		}

		void FillNoise()
		{
			g_SSAONoise.resize(c_NoiseSize);
			for(u32 i = 0; i < c_NoiseSize; i++)
			{
				g_SSAONoise[i] = glm::normalize(V3
				{
					Random::Float(0.0, 1.0) * 2.0 - 1.0,
					Random::Float(0.0, 1.0) * 2.0 - 1.0,
					0.0f
				});
			}
		}

		void CreateBuffer()
		{
			g_FrameBuffer = new OpenGLFrameBuffer(
				FrameBufferData{
				{
					FrameBufferTextureFormat::RED_INTEGER,	// Position buffer
				},
				1920,
				1080,
				1
				});
			g_BlurBuffer = new OpenGLFrameBuffer(
				FrameBufferData{
				{
					FrameBufferTextureFormat::RGBA16F,	// Position buffer
				},
				1920,
				1080,
				1
				});
		}

		void CreateNoiseTexture()
		{
			glGenTextures(1, &g_NoiseTexture);
			glBindTexture(GL_TEXTURE_2D, g_NoiseTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, g_SSAONoise.data());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			g_NoiseHandle = glGetTextureHandleARB(g_NoiseTexture);
			glMakeTextureHandleResidentARB(g_NoiseHandle);
		}

	}



	bool Initialize()
	{
		FillKernel();
		FillNoise();
		CreateBuffer();

		CreateNoiseTexture();

		{
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::SSAO));
			shader->Bind();

			g_SSAOUniforms.Projection = shader->GetUniformLocation("u_Projection");
			g_SSAOUniforms.Position = shader->GetUniformLocation("u_Position");
			g_SSAOUniforms.Normal = shader->GetUniformLocation("u_Normal");
			g_SSAOUniforms.Noise = shader->GetUniformLocation("u_Noise");
			g_SSAOUniforms.Size = shader->GetUniformLocation("u_KernelSize");
			g_SSAOUniforms.View = shader->GetUniformLocation("u_View");

			for(u32 i = 0; i < c_KernelSize; i++)
			{
				g_SamplesUniforms[i] = shader->GetUniformLocation("u_Samples[" + std::to_string(i) + "].Data");
			}

			shader->UnBind();
		}
		{
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(SSAO_BLUR));
			shader->Bind();
			g_SSAOTextureUniform = shader->GetUniformLocation("u_Texture");
			shader->UnBind();
		}
		return true;
	}

	void Update(Camera* camera, const OpenGLFrameBuffer& buffer)
	{
		{
			g_FrameBuffer->Bind();
			glClear(GL_COLOR_BUFFER_BIT);
			glViewport(0, 0, 1920, 1080);
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::SSAO));
			shader->Bind();

			for (u32 i = 0; i < c_KernelSize; i++)
			{
				shader->SetUniformRef(g_SamplesUniforms[i], g_SSAOKernel[i]);
			}
			shader->SetUniformRef(g_SSAOUniforms.Projection, camera->GetProjection());
			shader->SetUniform(g_SSAOUniforms.Size, c_KernelSize);
			shader->SetUniformRef(g_SSAOUniforms.View, camera->GetView());


			glUniformHandleui64ARB(g_SSAOUniforms.Position, buffer.ColorHandle());
			glUniformHandleui64ARB(g_SSAOUniforms.Normal, buffer.ColorHandle(1));
			glUniformHandleui64ARB(g_SSAOUniforms.Noise, g_NoiseHandle);

			RenderFullScreenQuad();

			shader->UnBind();
			g_FrameBuffer->UnBind();
		}
		{
			g_BlurBuffer->Bind();
			glClear(GL_COLOR_BUFFER_BIT);
			glViewport(0, 0, 1920, 1080);
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(SSAO_BLUR));
			shader->Bind();
			glUniformHandleui64ARB(g_SSAOTextureUniform, g_FrameBuffer->ColorHandle());

			RenderFullScreenQuad();

			shader->UnBind();
			g_BlurBuffer->UnBind();
		}
	}

	u64 GetSSAOHandle()
	{
		return g_BlurBuffer->ColorHandle();
	}

	u32 GetSSAOAttachment()
	{
		return g_BlurBuffer->ColorAttachment();
	}

	void Shutdown()
	{
		delete g_FrameBuffer;
		delete g_BlurBuffer;
	}
}
