#include <pch.h>
#include "OpenGLGridPass.h"

#include "Renderer/Camera.h"

#include "Renderer/OpenGL/OpenGLObjects/OpenGLFrameBuffer.h"
#include "Renderer/OpenGL/OpenGLObjects/OpenGLShader.h"

namespace Raito::Renderer::OpenGL::Grid
{
	namespace 
	{
		u32 g_GridQuadVAO;
		u32 g_GridQuadVBO;
	}
	bool Initialize()
	{
		constexpr float quadVertices[] = {
			// Positions        
			 0.0f, 0.0f, 0.0f,    
			-1.0f, 0.0f, 0.0f,    
			-1.0f, 1.0f, 0.0f,    
			 0.0f, 1.0f, 0.0f,    

			 0.0f, 0.0f, 0.0f,    
			 1.0f, 0.0f, 0.0f,    
			 1.0f, 1.0f, 0.0f,    
			 0.0f, 1.0f, 0.0f,    

			 0.0f, 0.0f, 0.0f,    
			-1.0f, 0.0f, 0.0f,    
			-1.0f, 0.0f, 1.0f,    
			 0.0f, 0.0f, 1.0f,    

			 0.0f, 0.0f, 0.0f,    
			 1.0f, 0.0f, 0.0f,    
			 1.0f, 0.0f, 1.0f,    
			 0.0f, 0.0f, 1.0f,    
		};

		glGenVertexArrays(1, &g_GridQuadVAO);
		glGenBuffers(1, &g_GridQuadVBO);
		glBindVertexArray(g_GridQuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, g_GridQuadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,  3 * sizeof(float), (void*)0);

		return true;
	}

	void Update(Camera* camera, const OpenGLFrameBuffer& buffer)
	{
		buffer.Bind();

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindVertexArray(g_GridQuadVAO);

		const auto shader = dynamic_cast<OpenGL::OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::GRID));
		shader->Bind();

		shader->SetUniform("u_Left", camera->GetLeft());
		shader->SetUniform("u_Right", camera->GetRight());
		shader->SetUniform("u_Bottom", camera->GetBottom());
		shader->SetUniform("u_Top", camera->GetTop());
		shader->SetUniform("u_ZNear", camera->GetNearClip());
		//shader->SetUniformRef("u_ViewMatrix", camera->GetView());

		glDrawArrays(GL_TRIANGLES, 0, 2);

		shader->UnBind();

		buffer.UnBind();
	}
}
