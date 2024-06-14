#include <pch.h>
#include "OpenGLSkyboxPass.h"

#include <Raito/Renderer/OpenGL/OpenGLCommon.h>
#include "Assets/AssetImport.h"
#include "Renderer/OpenGL/OpenGLCore.h"
#include "Renderer/OpenGL/OpenGLObjects/OpenGLShader.h"

namespace Raito::Renderer::OpenGL::Skybox
{
	namespace
	{
		u32 g_EnvCubemap;
		u32 g_IrradianceMap;
		u32 g_CaptureFBO;
		u32 g_CaptureRBO;
		u32 g_PrefilterMap;
		u32 g_BRDFLUTTMap;
		u64 g_BRDFLUTTHandle;




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
		



		void CreateCaptureFramebuffer()
		{
			glGenFramebuffers(1, &g_CaptureFBO);
			glGenRenderbuffers(1, &g_CaptureRBO);

			glBindFramebuffer(GL_FRAMEBUFFER, g_CaptureFBO);
			glBindRenderbuffer(GL_RENDERBUFFER, g_CaptureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_CaptureRBO);

			Assets::ImportTexture("Textures/autumn_field_puresky_4k.hdr", Assets::HDR);
			Assets::Texture* hdr = Assets::GetTexture("Textures/autumn_field_puresky_4k.hdr");

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

		void CreatePrefilterMap()
		{
			glGenTextures(1, &g_PrefilterMap);
			glBindTexture(GL_TEXTURE_CUBE_MAP, g_PrefilterMap);
			for (unsigned int i = 0; i < 6; ++i)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(PREFILTER));
			shader->Bind();
			shader->SetUniform("u_EnvironmentMap", 0);
			shader->SetUniformRef("u_Projection", g_CaptureProjection);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, g_EnvCubemap);
			glBindFramebuffer(GL_FRAMEBUFFER, g_CaptureFBO);
			constexpr u32 maxMipLevels = 5;
			for(u32 mip = 0; mip < maxMipLevels; mip++)
			{
				const u32 mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
				const u32 mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
				glBindRenderbuffer(GL_RENDERBUFFER, g_CaptureRBO);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
				glViewport(0, 0, mipWidth, mipHeight);

				float roughness = (float)mip / (float)(maxMipLevels - 1);
				shader->SetUniform("u_Roughness", roughness);
				for (unsigned int i = 0; i < 6; ++i)
				{
					shader->SetUniformRef("u_View", g_CaptureViews[i]);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, g_PrefilterMap, mip);

					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					RenderCube();
				}
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void CreateBRDFLUTTMap()
		{
			glGenTextures(1, &g_BRDFLUTTMap);

			// pre-allocate enough memory for the LUT texture.
			glBindTexture(GL_TEXTURE_2D, g_BRDFLUTTMap);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
			// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
			glBindFramebuffer(GL_FRAMEBUFFER, g_CaptureFBO);
			glBindRenderbuffer(GL_RENDERBUFFER, g_CaptureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_BRDFLUTTMap, 0);

			g_BRDFLUTTHandle = glGetTextureHandleARB(g_BRDFLUTTMap);
			glMakeTextureHandleResidentARB(g_BRDFLUTTHandle);

			glViewport(0, 0, 512, 512);
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(BRDF));
			shader->Bind();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			RenderFullScreenQuad();
			shader->UnBind();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, 1920, 1080);
		}

	}

	bool Initialize()
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		CreateCaptureFramebuffer();
		CreateIrradianceMap();
		CreatePrefilterMap();
		CreateBRDFLUTTMap();

		{
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(SKYBOX));
			shader->Bind();
			shader->SetUniform("u_EnvironmentMap", 0);
			shader->UnBind();
		}

		return true;
	}

	void Update(const Camera& camera)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
		glCullFace(GL_FRONT);
		const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(SKYBOX));
		shader->Bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, g_EnvCubemap);

		shader->SetUniformRef("u_Projection", camera.GetProjection());
		shader->SetUniformRef("u_View", camera.GetView());
		RenderCube();

		shader->UnBind();
		glCullFace(GL_BACK);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);

	}

	void Shutdown()
	{

	}

	u32 GetIrradianceMap()
	{
		return g_IrradianceMap;
	}

	u32 GetPrefilterMap()
	{
		return g_PrefilterMap;
	}

	u64 GetBRDFLUTTMap()
	{
		return g_BRDFLUTTHandle;
	}
}
