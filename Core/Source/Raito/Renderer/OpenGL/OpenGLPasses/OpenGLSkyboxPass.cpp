#include <pch.h>
#include "OpenGLSkyboxPass.h"

#include <Raito/Renderer/OpenGL/OpenGLCommon.h>
#include "Assets/AssetImport.h"
#include "Renderer/OpenGL/OpenGLObjects/OpenGLShader.h"

namespace Raito::Renderer::OpenGL::Skybox
{
	namespace
	{
		u32 g_EnvCubemap;
		u32 g_IrradianceMap;
		u32 g_CaptureFBO;
		u32 g_CaptureRBO;

		u32 g_CubeVAO;
		u32 g_CubeVBO;


		const glm::mat4 g_CaptureViews[] =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};
		const glm::mat4 g_CaptureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		void RenderCube()
		{
			// initialize (if necessary)
			if (g_CubeVAO == 0)
			{
				float vertices[] = {
					// back face
					-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
					 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
					 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
					 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
					-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
					-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
					// front face
					-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
					 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
					 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
					 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
					-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
					-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
					// left face
					-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
					-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
					-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
					-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
					-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
					-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
					// right face
					 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
					 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
					 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
					 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
					 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
					 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
					 // bottom face
					 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
					  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
					  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
					  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
					 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
					 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
					 // top face
					 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
					  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
					  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
					  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
					 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
					 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
				};
				glGenVertexArrays(1, &g_CubeVAO);
				glGenBuffers(1, &g_CubeVBO);
				// fill buffer
				glBindBuffer(GL_ARRAY_BUFFER, g_CubeVBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
				// link vertex attributes
				glBindVertexArray(g_CubeVAO);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}
			// render Cube
			glBindVertexArray(g_CubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		}



		void CreateCaptureFramebuffer()
		{
			glGenFramebuffers(1, &g_CaptureFBO);
			glGenRenderbuffers(1, &g_CaptureRBO);

			glBindFramebuffer(GL_FRAMEBUFFER, g_CaptureFBO);
			glBindRenderbuffer(GL_RENDERBUFFER, g_CaptureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_CaptureRBO);

			Assets::ImportTexture("Textures/thatch_chapel_4k.hdr", Assets::HDR);
			Assets::Texture* hdr = Assets::GetTexture("Textures/thatch_chapel_4k.hdr");

			if(!hdr)
			{
				O_ERROR("HDR not loaded");
				return;
			}
			{
				glGenTextures(1, &g_EnvCubemap);
				glBindTexture(GL_TEXTURE_CUBE_MAP, g_EnvCubemap);
				for (u32 i = 0; i < 6; ++i)
				{
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
				}
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EQUIRECTANGULAR_TO_CUBEMAP));
				shader->Bind();

				shader->SetUniformRef("u_Projection", g_CaptureProjection);
				glUniformHandleui64ARB(shader->GetUniformLocation("u_EquirectangularMap"), hdr->RenderData.Handle);

				glViewport(0, 0, 512, 512);
				glBindFramebuffer(GL_FRAMEBUFFER, g_CaptureFBO);
				for(u32 i = 0; i < 6;  i++)
				{
					shader->SetUniformRef("u_View", g_CaptureViews[i]);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, g_EnvCubemap, 0);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					RenderCube();
				}
				shader->UnBind();
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
		}



		void CreateIrradianceMap()
		{
			glGenTextures(1, &g_IrradianceMap);
			glBindTexture(GL_TEXTURE_CUBE_MAP, g_IrradianceMap);
			for (unsigned int i = 0; i < 6; ++i)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glBindFramebuffer(GL_FRAMEBUFFER, g_CaptureFBO);
			glBindRenderbuffer(GL_RENDERBUFFER, g_CaptureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

			{
				const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(IRRADIANCE));
				shader->Bind();
				shader->SetUniform("u_EnvironmentMap", 0);
				shader->SetUniformRef("u_Projection", g_CaptureProjection);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, g_EnvCubemap);

				glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
				glBindFramebuffer(GL_FRAMEBUFFER, g_CaptureFBO);
				for (unsigned int i = 0; i < 6; ++i)
				{
					shader->SetUniformRef("u_View", g_CaptureViews[i]);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, g_IrradianceMap, 0);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

					RenderCube();
				}
				shader->UnBind();
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	bool Initialize()
	{

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		CreateCaptureFramebuffer();
		CreateIrradianceMap();

		{
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(SKYBOX));
			shader->Bind();
			shader->SetUniform("u_EnvironmentMap", 0);
			shader->UnBind();
		}

		return true;
	}

	void Update(Camera* camera)
	{

		const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(SKYBOX));
		shader->Bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, g_EnvCubemap);
		shader->SetUniformRef("u_Projection", camera->GetProjection());
		shader->SetUniformRef("u_View", camera->GetView());
		RenderCube();

		shader->UnBind();
	}

	void Shutdown()
	{
	}

	u32 GetIrradianceMap()
	{
		return g_IrradianceMap;
	}
}
