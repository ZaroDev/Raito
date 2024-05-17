#include <pch.h>
#include "OpenGLDeferredPass.h"

#include "Raito.h"
#include "Assets/MeshGenerator.h"
#include "Core/Application.h"
#include "ECS/Components.h"
#include "Renderer/Camera.h"
#include "Renderer/OpenGL/OpenGLCore.h"


namespace Raito::Renderer::OpenGL::Deferred
{
	namespace
	{
		u32 g_FrameBufferQuadVAO, g_FrameBufferQuadVBO;

		constexpr u32 c_MaxPointLights = NUM_POINT;
		constexpr u32 c_MaxDirLights = NUM_DIRECTIONAL;

		struct DirectionalLightUniformLocations
		{
			GLint Position;
			GLint Color;
			GLint Direction;
		} g_DirectionalUniformLocations[c_MaxDirLights];

		struct PointLightUniformLocations
		{
			GLint Position;
			GLint Color;
			GLint Linear;
			GLint Quadratic;
			GLint Radius;
		} g_PointUniformLocations[c_MaxPointLights];

		GLint g_ViewPosLocation;
		GLint g_PointLightNumLocation;
		GLint g_DirectionalLightNumLocation;

		GLint g_PointLightCount = 0;
		GLint g_DirectionalLightCount = 0;

		OpenGLFrameBuffer* g_FrameBuffer = nullptr;
		OpenGLFrameBuffer* g_LightBuffer = nullptr;

	}

	bool Initialize()
	{
		g_FrameBuffer = new  OpenGLFrameBuffer(
			FrameBufferData{
				{
					FrameBufferTextureFormat::RGBA16F,	// Position buffer
					FrameBufferTextureFormat::RGBA16F,	// Normal buffer
					FrameBufferTextureFormat::RGBA16F,	// Albedo buffer
					FrameBufferTextureFormat::Depth		// Depth buffer
				},
				1920,
				1080,
				1
			});

		g_LightBuffer = new OpenGLFrameBuffer(
			FrameBufferData{
				{
					FrameBufferTextureFormat::RGBA16F,	// Emissive buffer
					FrameBufferTextureFormat::RGBA16F,	// Specular buffer
					},
				1920,
				1080,
				1
			});

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


		{
			const auto shader = dynamic_cast<OpenGL::OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::DEFERRED));
			shader->Bind();
			shader->SetUniform("u_Diffuse", 0);
			shader->SetUniform("u_Emissive", 1);
			shader->SetUniform("u_Specular", 2);
			shader->UnBind();
		}
		{
			const auto shader = dynamic_cast<OpenGL::OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::DEFERRED_POINT_LIGHT));
			shader->Bind();
			shader->SetUniform("u_Depth", 0);
			shader->SetUniform("u_Normal", 1);
			shader->UnBind();
		}

		{
			const auto shader = dynamic_cast<OpenGL::OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::DEFERRED_DIRECTIONAL_LIGHT));
			shader->Bind();
			shader->SetUniform("u_Depth", 0);
			shader->SetUniform("u_Normal", 1);
			shader->UnBind();
		}
		return true;
	}

	void Update(Camera* camera, const OpenGLFrameBuffer& buffer)
	{
		auto& scene = Core::Application::Get().Scene;

		// Geometry pass
		{
			g_FrameBuffer->Resize(buffer.Data().Width, buffer.Data().Height);
			g_FrameBuffer->Bind();


			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			const auto view = scene.GetAllEntitiesWith<ECS::TransformComponent, ECS::MeshComponent>();
			

			for (auto& entity : view)
			{
				const ECS::MeshComponent& mesh = view.get<ECS::MeshComponent>(entity);
				const OpenGLMeshData& meshData = GetMesh(mesh.MeshId);
				const Mat4 model = view.get<ECS::TransformComponent>(entity).GetTransform();
				const Mat3 normalMatrix = glm::transpose(glm::inverse(Mat3(model)));

				OpenGLMaterial& material = GetMaterial(mesh.MaterialId);


				material.SetValue("u_View", camera->GetView());
				material.SetValue("u_Projection", camera->GetProjection());
				material.SetValue("u_Model", model);
				material.SetValue("u_NormalMatrix", normalMatrix);

				material.Bind();

				glBindVertexArray(meshData.VAO);
				glDrawElements(meshData.RenderMode, meshData.IndexCount, GL_UNSIGNED_INT, nullptr);
				glBindVertexArray(0);

				material.UnBind();
			}
			g_FrameBuffer->UnBind();
		}
		// Light volumes pass
		{
			g_LightBuffer->Resize(buffer.Data().Width, buffer.Data().Height);
			g_LightBuffer->Bind();

			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			glBlendFunc(GL_ONE, GL_ONE);
			


			g_DirectionalLightCount = g_PointLightCount = 0;


			const auto lightView = scene.GetAllEntitiesWith<ECS::TransformComponent, ECS::LightComponent>();
			for (auto& light : lightView)
			{
				const ECS::LightComponent& lightCmp = lightView.get<ECS::LightComponent>(light);

				switch (lightCmp.LightType)
				{
				case ECS::LightComponent::Type::DIRECTIONAL:
				{
					const auto shader = dynamic_cast<OpenGL::OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::DEFERRED_DIRECTIONAL_LIGHT));
					shader->Bind();

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->DepthAttachment());
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->ColorAttachment(1));
					
					auto pixel = V3{ 1.0f / buffer.Data().Width, 1.0f / buffer.Data().Height, 0.0 };
					shader->SetUniformRef("u_CameraPos", camera->GetPosition());
					shader->SetUniformRef("u_PixelSize", pixel);
					
					shader->SetUniformRef("u_LightDirection", glm::radians(lightCmp.Direction));
					shader->SetUniformRef("u_LightColor", lightCmp.Color);
					
					
					glBindVertexArray(g_FrameBufferQuadVAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);
					shader->UnBind();

					g_DirectionalLightCount++;
				}break;
				case ECS::LightComponent::Type::POINT_LIGHT:
				{
					const auto shader = dynamic_cast<OpenGL::OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::DEFERRED_POINT_LIGHT));
					shader->Bind();

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->DepthAttachment());
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->ColorAttachment(1));

					auto pixel = V3{ 1.0f / buffer.Data().Width, 1.0f / buffer.Data().Height, 0.0 };
					shader->SetUniformRef("u_CameraPos", camera->GetPosition());
					shader->SetUniformRef("u_PixelSize", pixel);

					const auto& transform = lightView.get<ECS::TransformComponent>(light);
					const Mat4 model = transform.GetTransform();
					const OpenGLMeshData& meshData = GetMesh(Assets::GetDefaultSphere());

					shader->SetUniformRef("u_Model", model);
					shader->SetUniformRef("u_View", camera->GetView());
					shader->SetUniformRef("u_Projection", camera->GetProjection());
					
					shader->SetUniformRef("u_LightPos", transform.Translation);
					shader->SetUniformRef("u_LightColor", lightCmp.Color);
					shader->SetUniform("u_LightRadius", lightCmp.Radius);


					const float dist = glm::distance(transform.Translation, camera->GetPosition());
					if (dist < lightCmp.Radius)
					{
						glCullFace(GL_FRONT);
					}
					else
					{
						glCullFace(GL_BACK);
					}

					glBindVertexArray(meshData.VAO);
					glDrawElements(meshData.RenderMode, meshData.IndexCount, GL_UNSIGNED_INT, nullptr);
					glBindVertexArray(0);

					shader->UnBind();
					g_PointLightCount++;
				}break;
				case ECS::LightComponent::Type::SPOT_LIGHT:
					break;
				}
			}
			glCullFace(GL_BACK);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			g_LightBuffer->UnBind();
		}
		{
			buffer.Bind();
			const auto shader = dynamic_cast<OpenGL::OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::DEFERRED));
			shader->Bind();
			glDisable(GL_DEPTH_TEST);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->ColorAttachment(2));
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, g_LightBuffer->ColorAttachment());
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, g_LightBuffer->ColorAttachment(1));

			glBindVertexArray(g_FrameBufferQuadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			shader->UnBind();
			buffer.UnBind();
		}
	}

	void Shutdown()
	{
		glDeleteVertexArrays(1, &g_FrameBufferQuadVAO);
		glDeleteBuffers(1, &g_FrameBufferQuadVBO);
	}

	u32 GetDeferredAttachment(u32 id)
	{
		return g_FrameBuffer->ColorAttachment(id);
	}

	u32 GetDeferredDepth()
	{
		return g_FrameBuffer->DepthAttachment();
	}
}
