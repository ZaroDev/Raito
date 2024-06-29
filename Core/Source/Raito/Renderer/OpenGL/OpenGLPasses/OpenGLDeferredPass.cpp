#include <pch.h>
#include "OpenGLDeferredPass.h"

#include "Raito.h"
#include "OpenGLGeometryPass.h"
#include "OpenGLLightPass.h"
#include "Core/Application.h"
#include "optick/include/optick.h"
#include "Renderer/Camera.h"
#include "Renderer/OpenGL/OpenGLCore.h"

#include "OpenGLShadowPass.h"
#include "OpenGLSkyboxPass.h"
#include "OpenGLSSAOPass.h"

namespace Raito::Renderer::OpenGL::Deferred
{
	namespace
	{
		struct DirectionalLightUniformLocations
		{
			GLint Color;
			GLint Direction;
			GLint CameraPos;
			GLint PixelSize;
		} g_DirectionalUniformLocations;

		struct PointLightUniformLocations
		{
			GLint Position;
			GLint Color;
			GLint Radius;
			GLint CameraPos;
			GLint PixelSize;
			GLint View;
			GLint Projection;
			GLint Model;
		} g_PointUniformLocations;


		struct DirectionalUniform
		{
			GLint Direction;
			GLint Color;
		} g_DirectionalUniforms;


		struct GBufferUniforms
		{
			GLint Position;
			GLint Normal;
			GLint Albedo;
			GLint Emissive;
			GLint RoughMetal;
			GLint Ambient;
			GLint BRDFLUTT;
		} g_GBufferUniforms;


		std::unique_ptr<OpenGLFrameBuffer> g_FrameBuffer = nullptr;
		OpenGLShader* g_GBuffer = nullptr;

	}

	bool Initialize()
	{
		g_FrameBuffer = std::make_unique<OpenGLFrameBuffer>(
			FrameBufferData{
				{
					FrameBufferTextureFormat::RGBA16F,// Position buffer
					FrameBufferTextureFormat::RGBA16F,// Normal buffer
					FrameBufferTextureFormat::RGBA,	// Albedo buffer
					FrameBufferTextureFormat::RGBA,	// Emissive buffer
					FrameBufferTextureFormat::RGBA,	// RougMetalAO buffer
					FrameBufferTextureFormat::Depth	// Depth buffer
				},
				1920,
				1080,
				1
			});

		{

			g_GBuffer = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(G_BUFFER));
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DEFERRED));
			shader->Bind();
			
			shader->SetUniform("u_ShadowMap", 0);
			shader->SetUniform("u_IrradianceMap", 1);
			shader->SetUniform("u_PrefilterMap", 2);


			g_GBufferUniforms.Position = shader->GetUniformLocation("u_GPosition");
			g_GBufferUniforms.Normal = shader->GetUniformLocation("u_GNormal");
			g_GBufferUniforms.Albedo = shader->GetUniformLocation("u_GAlbedo");
			g_GBufferUniforms.Emissive = shader->GetUniformLocation("u_GEmissive");
			g_GBufferUniforms.RoughMetal = shader->GetUniformLocation("u_GRoughMetalAO");
			g_GBufferUniforms.Ambient = shader->GetUniformLocation("u_SSAO");
			g_GBufferUniforms.BRDFLUTT = shader->GetUniformLocation("u_BRDFLUT");
			shader->UnBind();
		}
		{
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DEFERRED_POINT_LIGHT));
			shader->Bind();
			shader->SetUniform("u_Depth", 0);
			shader->SetUniform("u_Normal", 1);

			g_PointUniformLocations.View = shader->GetUniformLocation("u_View");
			g_PointUniformLocations.Model = shader->GetUniformLocation("u_Model");
			g_PointUniformLocations.Projection = shader->GetUniformLocation("u_Projection");
			g_PointUniformLocations.CameraPos = shader->GetUniformLocation("u_CameraPos");
			g_PointUniformLocations.PixelSize = shader->GetUniformLocation("u_PixelSize");
			g_PointUniformLocations.Position = shader->GetUniformLocation("u_LightPos");
			g_PointUniformLocations.Color = shader->GetUniformLocation("u_LightColor");
			g_PointUniformLocations.Radius = shader->GetUniformLocation("u_LightRadius");


			shader->UnBind();
		}

		{
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DEFERRED_DIRECTIONAL_LIGHT));
			shader->Bind();
			shader->SetUniform("u_Depth", 0);
			shader->SetUniform("u_Normal", 1);

			g_DirectionalUniformLocations.CameraPos = shader->GetUniformLocation("u_CameraPos");
			g_DirectionalUniformLocations.PixelSize = shader->GetUniformLocation("u_PixelSize");
			g_DirectionalUniformLocations.Color = shader->GetUniformLocation("u_LightColor");
			g_DirectionalUniformLocations.Direction = shader->GetUniformLocation("u_LightDirection");
			shader->UnBind();
		}


		
		return true;
	}


	void Update(const Camera& camera, const OpenGLFrameBuffer& buffer)
	{
		auto& scene = Core::Application::Get().Scene;
		OPTICK_CATEGORY("Update Deferred", Optick::Category::Rendering);
		// Geometry pass
		{
			g_FrameBuffer->Bind();
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glDepthFunc(GL_LESS);
		  
			Geometry::Update(scene, camera, buffer, g_GBuffer);


			g_FrameBuffer->UnBind();
		}

		// SSAO
		{
			SSAO::Update(camera, *g_FrameBuffer);
		}
		// Combine pass
		{
			buffer.Bind();
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DEFERRED));
			shader->Bind();
			glDisable(GL_DEPTH_TEST);

			glUniformHandleui64ARB(g_GBufferUniforms.Position, g_FrameBuffer->ColorHandle());
			glUniformHandleui64ARB(g_GBufferUniforms.Normal, g_FrameBuffer->ColorHandle(1));
			glUniformHandleui64ARB(g_GBufferUniforms.Albedo, g_FrameBuffer->ColorHandle(2));
			glUniformHandleui64ARB(g_GBufferUniforms.Emissive, g_FrameBuffer->ColorHandle(3));
			glUniformHandleui64ARB(g_GBufferUniforms.RoughMetal, g_FrameBuffer->ColorHandle(4));
			glUniformHandleui64ARB(g_GBufferUniforms.Ambient, SSAO::GetSSAOHandle());
			glUniformHandleui64ARB(g_GBufferUniforms.BRDFLUTT, Skybox::GetBRDFLUTTMap());

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Shadows::GetShadowMap());

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::GetIrradianceMap());

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::GetPrefilterMap());

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, Shadows::GetShadowMapSSBO());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, Shadows::GetShadowMapSSBO());

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, Shadows::GetLightMatricesSSBO());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, Shadows::GetLightMatricesSSBO());

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, LightPass::GetDirectionalSSBO());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, LightPass::GetDirectionalSSBO());

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, LightPass::GetPointSSBO());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, LightPass::GetPointSSBO());

			
			shader->SetUniformRef("u_ViewPosition",camera.GetPosition());
			shader->SetUniform("u_PointSize", LightPass::GetPointSize());
			shader->SetUniformRef("u_View", camera.GetView());


			RenderFullScreenQuad();

			shader->UnBind();
			buffer.UnBind();

			glBindBuffer(GL_UNIFORM_BUFFER, 0);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}
	}

	void Shutdown()
	{
	}

	u32 GetDeferredAttachment(u32 id)
	{
		return g_FrameBuffer->ColorAttachment(id);
	}

	u32 GetLightAttachment(u32 id)
	{
		return 0;
	}

	u32 GetDeferredDepth()
	{
		return g_FrameBuffer->DepthAttachment();
	}
}
