#include <pch.h>
#include "OpenGLDeferredPlus.h"

#include "OpenGLGeometryPass.h"
#include "OpenGLLightPass.h"
#include "OpenGLShadowPass.h"
#include "OpenGLSkyboxPass.h"
#include "OpenGLSSAOPass.h"
#include "Core/Application.h"
#include "ECS/Components.h"
#include "Renderer/OpenGL/OpenGLCore.h"
#include "Renderer/OpenGL/OpenGLObjects/OpenGLShader.h"

namespace Raito::Renderer::OpenGL::DeferredPlus
{
	namespace
	{
		constexpr u32 g_MaxObjectBufferSize = 2048;
		std::unique_ptr<OpenGLFrameBuffer> g_FrameBuffer = nullptr;
		std::unique_ptr<OpenGLFrameBuffer> g_LightBuffer = nullptr;

		struct Buffers
		{
			GLuint PointLightModel;
			GLuint PointLight;
			GLuint DirectionalLight;
		} g_Buffers;



		void GenerateBuffer(u32& id, GLenum type, size_t size, GLenum flags)
		{
			glGenBuffers(1, &id);
			glBindBuffer(type, id);
			glBufferData(type, size, nullptr, flags);
			glBindBuffer(type, 0);
		}

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
					FrameBufferTextureFormat::Depth		// Depth buffer
				},
				1920,
				1080,
				1
			});

		g_LightBuffer = std::make_unique<OpenGLFrameBuffer>(
			FrameBufferData{
					{
						FrameBufferTextureFormat::RGBA,	// Directional buffer
						FrameBufferTextureFormat::RGBA,	// Point buffer
						FrameBufferTextureFormat::RGBA,	// Irradiance buffer
					},
					1920,
					1080,
					1
			});


		GenerateBuffer(g_Buffers.PointLightModel, GL_SHADER_STORAGE_BUFFER, sizeof(Mat4) * g_MaxObjectBufferSize, GL_DYNAMIC_DRAW);
		GenerateBuffer(g_Buffers.DirectionalLight, GL_SHADER_STORAGE_BUFFER, sizeof(Mat3), GL_DYNAMIC_DRAW);
		GenerateBuffer(g_Buffers.PointLight, GL_SHADER_STORAGE_BUFFER, sizeof(Mat3) * g_MaxObjectBufferSize, GL_DYNAMIC_DRAW);



		{
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DRAW_COMMAND));
			shader->Bind();
			const uint32_t storageBlockIndex = glGetProgramResourceIndex(shader->Id(), GL_SHADER_STORAGE_BLOCK, "DrawCommandsBlock");
			glShaderStorageBlockBinding(shader->Id(), storageBlockIndex, 1);

			shader->UnBind();
		}

		{
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DEFERRED_AMBIENT));
			shader->Bind();

			shader->SetUniform("u_IrradianceMap", 0);
			shader->SetUniform("u_PrefilterMap", 1);
		}


		return true;
	}

	void Update(const Camera& camera, const OpenGLFrameBuffer& buffer)
	{
		const auto& scene = Core::Application::Get().Scene;

		// Fill the g-buffer
		{
			g_FrameBuffer->Bind();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glCullFace(GL_BACK);
			glViewport(0, 0, 1920, 1080);
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(G_BUFFER));
			Geometry::Update(scene, camera, *g_FrameBuffer, shader);

			shader->UnBind();
			g_FrameBuffer->UnBind();
		}
		
		SSAO::Update(camera, *g_FrameBuffer);

		{
			g_LightBuffer->Bind();
			glViewport(0, 0, 1920, 1080);
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			const auto view = scene.GetAllEntitiesWith<ECS::TransformComponent, ECS::LightComponent>();
			std::vector<Mat4> matrices;
			std::vector<Mat3> points;
			for (const auto& entity : view)
			{
				const auto& light = view.get<ECS::LightComponent>(entity);
				const auto& transform = view.get<ECS::TransformComponent>(entity);

				switch (light.LightType)
				{
				case ECS::LightComponent::Type::DIRECTIONAL:
				{

					const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DEFERRED_DIRECTIONAL_LIGHT));
					shader->Bind();
					SetTextureOnShader("u_GPosition", *shader, { g_FrameBuffer->ColorHandle(), 0 });
					SetTextureOnShader("u_GNormal", *shader, { g_FrameBuffer->ColorHandle(1), 0 });
					SetTextureOnShader("u_GAlbedo", *shader, { g_FrameBuffer->ColorHandle(2), 0 });
					SetTextureOnShader("u_GRoughMetalAO", *shader, { g_FrameBuffer->ColorHandle(4), 0 });
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D_ARRAY, Shadows::GetShadowMap());

					glBindBuffer(GL_SHADER_STORAGE_BUFFER, Shadows::GetShadowMapSSBO());
					glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, Shadows::GetShadowMapSSBO());

					glBindBuffer(GL_SHADER_STORAGE_BUFFER, Shadows::GetLightMatricesSSBO());
					glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, Shadows::GetLightMatricesSSBO());

					const auto dir = Mat3(light.Direction, light.Color, V3{ 0.0f });
					glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_Buffers.DirectionalLight);
					glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Mat3), &dir);
					glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, g_Buffers.DirectionalLight);

					shader->SetUniformRef("u_View", camera.GetView());

					RenderFullScreenQuad();


				}break;
				case ECS::LightComponent::Type::POINT_LIGHT:
				{
					const auto point = Mat3(transform.Translation, light.Color, V3{ 0.f });
					matrices.emplace_back(transform.GetTransform());
					points.emplace_back(point);
				}break;
				case ECS::LightComponent::Type::SPOT_LIGHT:
				{
				}
				break;
				}
			}

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_Buffers.PointLightModel);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Mat4)* matrices.size(), matrices.data());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_Buffers.PointLightModel);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_Buffers.PointLight);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_Buffers.PointLight);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Mat3)* points.size(), points.data());
			{
				

				const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DEFERRED_POINT_LIGHT));
				shader->Bind();
				SetTextureOnShader("u_GPosition", *shader, { g_FrameBuffer->ColorHandle(), 0 });
				SetTextureOnShader("u_GNormal", *shader, { g_FrameBuffer->ColorHandle(1), 0 });
				SetTextureOnShader("u_GAlbedo", *shader, { g_FrameBuffer->ColorHandle(2), 0 });
				SetTextureOnShader("u_GRoughMetalAO", *shader, { g_FrameBuffer->ColorHandle(4), 0 });


				glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_Buffers.PointLightModel);
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Mat4)* matrices.size(), matrices.data());

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_Buffers.PointLight);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_Buffers.PointLight);

				shader->SetUniformRef("u_ScreenSize", V2{ 1920, 1080 });
				shader->SetUniformRef("u_ViewPosition", camera.GetPosition());

				shader->SetUniformRef("u_View", camera.GetView());
				shader->SetUniformRef("u_Projection", camera.GetProjection());


				glCullFace(GL_FRONT);
				RenderSphereInstanced(matrices.size());

				shader->UnBind();
				glCullFace(GL_BACK);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			{
				const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DEFERRED_AMBIENT));
				shader->Bind();

				SetTextureOnShader("u_GPosition", *shader, { g_FrameBuffer->ColorHandle(), 0 });
				SetTextureOnShader("u_GNormal", *shader, { g_FrameBuffer->ColorHandle(1), 0 });
				SetTextureOnShader("u_GAlbedo", *shader, { g_FrameBuffer->ColorHandle(2), 0 });
				SetTextureOnShader("u_GRoughMetalAO", *shader, { g_FrameBuffer->ColorHandle(4), 0 });
				SetTextureOnShader("u_SSAO", *shader, { SSAO::GetSSAOHandle(), 0 });
				SetTextureOnShader("u_BRDFLUT", *shader, { Skybox::GetBRDFLUTTMap(), 0 });

				shader->SetUniformRef("u_ViewPosition", camera.GetPosition());


				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::GetIrradianceMap());

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::GetPrefilterMap());

				RenderFullScreenQuad();

				shader->UnBind();
			}
			g_LightBuffer->UnBind();
		}


		{
			buffer.Bind();
			glClear(GL_COLOR_BUFFER_BIT);
			glDisable(GL_DEPTH_TEST);
			glViewport(0, 0, buffer.Data().Width, buffer.Data().Height);
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DEFERRED_PLUS_COMBINE));
			shader->Bind();



			SetTextureOnShader("u_Directional", *shader, { g_LightBuffer->ColorHandle(), 0 });
			SetTextureOnShader("u_Point", *shader, { g_LightBuffer->ColorHandle(1), 0 });
			SetTextureOnShader("u_Ambient", *shader, { g_LightBuffer->ColorHandle(2), 0 });
			SetTextureOnShader("u_Emissive", *shader, { g_FrameBuffer->ColorHandle(3), 0 });

			RenderFullScreenQuad();
			buffer.UnBind();
		}
	}

	u32 GetDeferredAttachment(u32 id)
	{
		return g_FrameBuffer->ColorAttachment(id);
	}

	u32 GetDeferredDepth()
	{
		return g_FrameBuffer->DepthAttachment();
	}

	u32 GetLightAttachment(u32 id)
	{
		return g_LightBuffer->ColorAttachment(id);
	}

	void Shutdown()
	{
	}
}
