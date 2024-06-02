#include <pch.h>
#include "OpenGLSSAOPass.h"

#include "Random/Random.h"
#include "Renderer/OpenGL/OpenGLObjects/OpenGLShader.h"
#include <Raito/Math/Math.h>

namespace Raito::Renderer::OpenGL::SSAO
{
	namespace
	{
		u32 g_SSAOBuffer;
		u32 g_SSAOTexture;
		u64 g_SSAOHandle;

		u32 g_SSAOBlurBuffer;
		u32 g_SSAOBlurTexture;
		u64 g_SSAOBlurHandle;

		constexpr u32 g_KernelSize = 64;
		std::vector<V3> g_SSAOKernel;

		constexpr u32 g_NoiseSize = 16;
		std::vector<V3> g_SSAONoise;

		void FillKernel()
		{
			g_SSAOKernel.reserve(g_KernelSize);
			for(u32 i = 0; i < g_KernelSize; i++)
			{
				V3 sample = V3
				{
					Random::Float(0.0, 1.0) * 2.0 - 1.0,
					Random::Float(0.0, 1.0) * 2.0 - 1.0,
					Random::Float(0.0, 1.0)
				};

				sample *= glm::normalize(sample);
				sample *= Random::Float(0.0, 1.0);

				float scale = static_cast<float>(i) / static_cast<float>(g_KernelSize);

				scale = Math::Lerp(0.1f, 1.0f, scale * scale);
				sample *= scale;
				g_SSAOKernel[i] = sample;
			}
		}

		void FillNoise()
		{
			g_SSAONoise.reserve(g_NoiseSize);
			for(u32 i = 0; i < g_NoiseSize; i++)
			{
				g_SSAONoise[i] = V3
				{
					Random::Float(0.0, 1.0) * 2.0 - 1.0,
					Random::Float(0.0, 1.0) * 2.0 - 1.0,
					0.0f
				};
			}
		}

		void CreateBuffer()
		{
			glGenFramebuffers(1, &g_SSAOBuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, g_SSAOBuffer);
			glGenTextures(1, &g_SSAOTexture);
			glBindTexture(GL_TEXTURE_2D, g_SSAOTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1920, 1080, 0, GL_RED, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

	}

	bool Initialize()
	{
		FillKernel();
		FillNoise();

		return true;
	}

	void Update(Camera* camera, const OpenGLFrameBuffer& buffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, g_SSAOBuffer);
		glClear(GL_COLOR_BUFFER_BIT);
		const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::SSAO));
		shader->Bind();

		for(u32 i = 0; i < 64; i++)
		{
			
		}

		shader->UnBind();
	}

	void Shutdown()
	{
	}
}
