#include <pch.h>
#include "OpenGLDeferredPass.h"

#include "Raito.h"
#include "Core/Application.h"
#include "ECS/Components.h"
#include "optick/include/optick.h"
#include "Renderer/Camera.h"
#include "Renderer/OpenGL/OpenGLCore.h"

#include "OpenGLShadowPass.h"

namespace Raito::Renderer::OpenGL::Deferred
{
	namespace
	{
		u32 g_FrameBufferQuadVAO, g_FrameBufferQuadVBO;

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

		struct PointUniform
		{
			GLint Position;
			GLint Color;
		} g_PointUniforms[NUM_POINT];

		GLint g_LightCount = 0;

		GLint g_LightCountUniform;
		GLint g_InvProjectionUniform;

		OpenGLFrameBuffer* g_FrameBuffer = nullptr;
		OpenGLFrameBuffer* g_LightBuffer = nullptr;

		Shadows::CascadeUniforms g_CascadeUniforms;

	}

	bool Initialize()
	{
		g_FrameBuffer = new  OpenGLFrameBuffer(
			FrameBufferData{
				{
					FrameBufferTextureFormat::RGBA16F,	// Position buffer
					FrameBufferTextureFormat::RGBA16F,	// Normal buffer
					FrameBufferTextureFormat::RGBA16F,	// Albedo buffer
					FrameBufferTextureFormat::RGBA16F,	// Emissive buffer
					FrameBufferTextureFormat::RGBA16F,	// RougMetalAO buffer
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
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DEFERRED));
			shader->Bind();
			shader->SetUniform("u_GPosition", 0);
			shader->SetUniform("u_GNormal", 1);
			shader->SetUniform("u_GAlbedo", 2);
			shader->SetUniform("u_GEmissive", 3);
			shader->SetUniform("u_GRoughMetalAO", 4);
			shader->SetUniform("u_ShadowMap", 5);

			g_DirectionalUniforms.Color = shader->GetUniformLocation("u_Directional.Color");
			g_DirectionalUniforms.Direction = shader->GetUniformLocation("u_Directional.Direction");

			for(u32 i = 0; i < NUM_POINT; i++)
			{
				
			}

			g_CascadeUniforms.CascadeCount = shader->GetUniformLocation("u_ShadowMapData.Size");
			g_CascadeUniforms.FarPlane = shader->GetUniformLocation("u_ShadowMapData.FarPlane");
			g_CascadeUniforms.View = shader->GetUniformLocation("u_View");

			g_LightCountUniform = shader->GetUniformLocation("u_NumLights");
			g_InvProjectionUniform = shader->GetUniformLocation("u_InvProjection");
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

	void Update(Camera* camera, const OpenGLFrameBuffer& buffer)
	{
		auto& scene = Core::Application::Get().Scene;
		OPTICK_CATEGORY("Update Deferred", Optick::Category::Rendering);
		// Geometry pass
		{
			g_FrameBuffer->Resize(buffer.Data().Width, buffer.Data().Height);
			g_FrameBuffer->Bind();


			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glDepthFunc(GL_LESS);
			glViewport(0, 0, buffer.Data().Width, buffer.Data().Height);

			const auto view = scene.GetAllEntitiesWith<ECS::TransformComponent, ECS::MeshComponent>();


			for (auto& entity : view)
			{
				const ECS::MeshComponent& mesh = view.get<ECS::MeshComponent>(entity);
				const OpenGLMeshData& meshData = GetMesh(mesh.MeshId);
				const auto& transform = view.get<ECS::TransformComponent>(entity);
				const Mat4 model = transform.GetTransform();
				const V3 location = transform.Translation;

				// Construct a AABB with the model translation
				//const auto aabb = Math::AABB(meshData.AABB.GetMin() + location, meshData.AABB.GetMin() + location);


				/*if(!camera->IsInsideFrustum(aabb))
				{
					continue;
				}*/


				const Mat3 normalMatrix = transpose(glm::inverse(Mat3(model)));
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
		{
			//// Light volumes pass
			//{
			//	g_LightBuffer->Resize(buffer.Data().Width, buffer.Data().Height);
			//	g_LightBuffer->Bind();

			//	glClearColor(0, 0, 0, 1);
			//	glClear(GL_COLOR_BUFFER_BIT);
			//	glBlendFunc(GL_ONE, GL_ONE);

			//	g_LightCount = 0;

			//	const auto lightView = scene.GetAllEntitiesWith<ECS::TransformComponent, ECS::LightComponent>();
			//	for (auto& light : lightView)
			//	{
			//		const ECS::LightComponent& lightCmp = lightView.get<ECS::LightComponent>(light);

			//		switch (lightCmp.LightType)
			//		{
			//		case ECS::LightComponent::Type::DIRECTIONAL:
			//		{
			//			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DEFERRED_DIRECTIONAL_LIGHT));
			//			shader->Bind();

			//			glActiveTexture(GL_TEXTURE0);
			//			glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->DepthAttachment());
			//			glActiveTexture(GL_TEXTURE1);
			//			glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->ColorAttachment(1));
			//			
			//			auto pixel = V3{ 1.0f / buffer.Data().Width, 1.0f / buffer.Data().Height, 0.0 };
			//			shader->SetUniformRef(g_DirectionalUniformLocations.CameraPos, camera->GetPosition());
			//			shader->SetUniformRef(g_DirectionalUniformLocations.PixelSize, pixel);
			//			
			//			shader->SetUniformRef(g_DirectionalUniformLocations.Direction, glm::radians(lightCmp.Direction));
			//			shader->SetUniformRef(g_DirectionalUniformLocations.Color, lightCmp.Color);
			//			
			//			
			//			glBindVertexArray(g_FrameBufferQuadVAO);
			//			glDrawArrays(GL_TRIANGLES, 0, 6);
			//			shader->UnBind();

			//		}break;
			//		case ECS::LightComponent::Type::POINT_LIGHT:
			//		{
			//			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DEFERRED_POINT_LIGHT));
			//			shader->Bind();

			//			glActiveTexture(GL_TEXTURE0);
			//			glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->DepthAttachment());
			//			glActiveTexture(GL_TEXTURE1);
			//			glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->ColorAttachment(1));

			//			auto pixel = V3{ 1.0f / buffer.Data().Width, 1.0f / buffer.Data().Height, 0.0 };
			//			shader->SetUniformRef(g_PointUniformLocations.CameraPos, camera->GetPosition());
			//			shader->SetUniformRef(g_PointUniformLocations.PixelSize, pixel);

			//			const auto& transform = lightView.get<ECS::TransformComponent>(light);
			//			const Mat4 model = transform.GetTransform();
			//			const OpenGLMeshData& meshData = GetMesh(Assets::GetDefaultSphere());

			//			shader->SetUniformRef(g_PointUniformLocations.Model, model);
			//			shader->SetUniformRef(g_PointUniformLocations.View, camera->GetView());
			//			shader->SetUniformRef(g_PointUniformLocations.Projection, camera->GetProjection());
			//			
			//			shader->SetUniformRef(g_PointUniformLocations.Position, transform.Translation);
			//			shader->SetUniformRef(g_PointUniformLocations.Color, lightCmp.Color);
			//			shader->SetUniform(g_PointUniformLocations.Radius, lightCmp.Radius);


			//			const float dist = glm::distance(transform.Translation, camera->GetPosition());
			//			if (dist < lightCmp.Radius)
			//			{
			//				glCullFace(GL_FRONT);
			//			}
			//			else
			//			{
			//				glCullFace(GL_BACK);
			//			}

			//			glBindVertexArray(meshData.VAO);
			//			glDrawElements(meshData.RenderMode, meshData.IndexCount, GL_UNSIGNED_INT, nullptr);
			//			glBindVertexArray(0);

			//			shader->UnBind();
			//		}break;
			//		case ECS::LightComponent::Type::SPOT_LIGHT:
			//			break;
			//		}
			//		g_LightCount++;
			//	}
			//	glCullFace(GL_BACK);
			//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			//	g_LightBuffer->UnBind();
			//}
		}
		{


			buffer.Bind();
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DEFERRED));
			shader->Bind();
			glDisable(GL_DEPTH_TEST);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->ColorAttachment());

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->ColorAttachment(1));

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->ColorAttachment(2));

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->ColorAttachment(3));

			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->ColorAttachment(4));

			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Shadows::GetShadowMap());

			

			glBindBuffer(GL_UNIFORM_BUFFER, Shadows::GetLightSpaceMatricesUBO());

			const auto& cascadeLevels = Shadows::GetCascadeLevels();
			for (u32 i = 0; i < cascadeLevels.size(); i++)
			{
				shader->SetUniform<float>(std::string("u_ShadowMapData.CascadePlanes[" + std::to_string(i) + "].Plane").c_str(), cascadeLevels[i]);
			}
			shader->SetUniform(g_CascadeUniforms.CascadeCount, static_cast<i32>(cascadeLevels.size()));
			shader->SetUniform(g_CascadeUniforms.FarPlane, camera->GetNearFar());


			g_LightCount = 0;

			const auto lightView = scene.GetAllEntitiesWith<ECS::TransformComponent, ECS::LightComponent>();
			for (auto& light : lightView)
			{
				const ECS::LightComponent& lightCmp = lightView.get<ECS::LightComponent>(light);
				const ECS::TransformComponent& transform = lightView.get<ECS::TransformComponent>(light);
				switch (lightCmp.LightType)
				{
				case ECS::LightComponent::Type::DIRECTIONAL:
				{
					
					shader->SetUniformRef(g_DirectionalUniforms.Direction, lightCmp.Direction);
					shader->SetUniformRef(g_DirectionalUniforms.Color, lightCmp.Color);
				}break;
				case ECS::LightComponent::Type::POINT_LIGHT:
				{
				}break;
				case ECS::LightComponent::Type::SPOT_LIGHT:
					break;
				}
				g_LightCount++;
			}
			shader->SetUniformRef(g_CascadeUniforms.View, camera->GetView());
			shader->SetUniformRef("u_ViewPos", camera->GetPosition());


			//shader->SetUniform(g_LightCountUniform, g_LightCount);
			//shader->SetUniformRef(g_InvProjectionUniform, camera->GetInverseProjection());


			glBindVertexArray(g_FrameBufferQuadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			shader->UnBind();
			buffer.UnBind();

			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
	}

	void Shutdown()
	{
		glDeleteVertexArrays(1, &g_FrameBufferQuadVAO);
		glDeleteBuffers(1, &g_FrameBufferQuadVBO);

		delete g_LightBuffer;
		delete g_FrameBuffer;
	}

	u32 GetDeferredAttachment(u32 id)
	{
		return g_FrameBuffer->ColorAttachment(id);
	}

	u32 GetLightAttachment(u32 id)
	{
		return g_LightBuffer->ColorAttachment(id);
	}

	u32 GetDeferredDepth()
	{
		return g_FrameBuffer->DepthAttachment();
	}
}
