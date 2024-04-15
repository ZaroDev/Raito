#include <pch.h>
#include "OpenGLPostProcessPass.h"

#include "Renderer/Camera.h"
#include "Renderer/OpenGL/OpenGLObjects/OpenGLShader.h"

namespace Raito::Renderer::OpenGL::PostProcess
{
	namespace
	{
		u32 g_FrameBufferQuadVAO, g_FrameBufferQuadVBO;
	}


	bool Initialize()
	{
		constexpr float quadVertices[] = {
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			1.0f, -1.0f,  1.0f, 0.0f,
			1.0f,  1.0f,  1.0f, 1.0f
		};

		glGenVertexArrays(1, &g_FrameBufferQuadVAO);
		glGenBuffers(1, &g_FrameBufferQuadVBO);
		glBindVertexArray(g_FrameBufferQuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, g_FrameBufferQuadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

		return true;
	}

	void Update(const OpenGLFrameBuffer& buffer)
	{

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		const auto shader = dynamic_cast<OpenGL::OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::POST_PROCESS));

		shader->Bind();

		glBindVertexArray(g_FrameBufferQuadVAO);
		glDisable(GL_DEPTH_TEST);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, buffer.ColorAttachment(3));

		glDrawArrays(GL_TRIANGLES, 0, 6);

		shader->UnBind();


	}

	void Shutdown()
	{
		glDeleteVertexArrays(1, &g_FrameBufferQuadVAO);
		glDeleteBuffers(1, &g_FrameBufferQuadVBO);
	}
}
