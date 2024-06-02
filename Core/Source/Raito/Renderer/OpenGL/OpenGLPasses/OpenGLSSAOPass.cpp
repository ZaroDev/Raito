#include <pch.h>
#include "OpenGLSSAOPass.h"

#include "Renderer/OpenGL/OpenGLObjects/OpenGLShader.h"

namespace Raito::Renderer::OpenGL::SSAO
{
	namespace
	{
		u32 g_SSAOBuffer;
		u32 g_SSAOBlurBuffer;
		u32 g_SSAOTexture;
		u64 g_SSAOHandle;

		

		void CreateBuffer()
		{
			
		}

	}

	bool Initialize()
	{
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
