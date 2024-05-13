#include <pch.h>
#include "OpenGLDeferredPass.h"

#include "Core/Application.h"
#include "ECS/Components.h"
#include "Renderer/Camera.h"
#include "Renderer/OpenGL/OpenGLCore.h"


namespace Raito::Renderer::OpenGL::Deferred
{
	namespace
	{
		u32 g_FrameBufferQuadVAO, g_FrameBufferQuadVBO;

		constexpr u32 c_MaxLights = 32;

		struct LightUniformLocations
		{
			GLint Position;
			GLint Color;
			GLint Linear;
			GLint Quadratic;
			GLint Radius;
		} g_UniformLocations[c_MaxLights];

		GLint g_ViewPosLocation;
		GLint g_LightNumLocation;

		GLint g_LightCount = 0;

		OpenGLFrameBuffer* g_FrameBuffer = nullptr;

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
			shader->SetUniform("gPosition", 0);
			shader->SetUniform("gNormal", 1);
			shader->SetUniform("gAlbedoSpec", 2);

			for (u32 i = 0; i < c_MaxLights; i++)
			{
				g_UniformLocations[i].Position = shader->GetUniformLocation(std::string("u_Lights[" + std::to_string(i) + "].Position"));
				g_UniformLocations[i].Color = shader->GetUniformLocation(std::string("u_Lights[" + std::to_string(i) + "].Color"));
				g_UniformLocations[i].Linear = shader->GetUniformLocation(std::string("u_Lights[" + std::to_string(i) + "].Linear"));
				g_UniformLocations[i].Quadratic = shader->GetUniformLocation(std::string("u_Lights[" + std::to_string(i) + "].Quadratic"));
				g_UniformLocations[i].Radius = shader->GetUniformLocation(std::string("u_Lights[" + std::to_string(i) + "].Radius"));
			}

			g_ViewPosLocation = shader->GetUniformLocation("u_ViewPos");
			g_LightNumLocation = shader->GetUniformLocation("u_LightsNum");
			shader->UnBind();
		}

		return true;
	}

	void Update(Camera* camera, const OpenGLFrameBuffer& buffer)
	{
		g_FrameBuffer->Resize(buffer.Data().Width, buffer.Data().Height);
		g_FrameBuffer->Bind();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

		auto& scene = Core::Application::Get().Scene;
		const auto view = scene.GetAllEntitiesWith<ECS::TransformComponent, ECS::MeshComponent>();
		const auto lightView = scene.GetAllEntitiesWith<ECS::TransformComponent, ECS::LightComponent>();

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
			glDrawElements(GL_TRIANGLES, meshData.IndexCount, GL_UNSIGNED_INT, nullptr);
			glBindVertexArray(0);

			material.UnBind();
		}
		g_FrameBuffer->UnBind();

		buffer.Bind();

		glBindVertexArray(g_FrameBufferQuadVAO);
		glDisable(GL_DEPTH_TEST);

		const auto shader = dynamic_cast<OpenGL::OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::DEFERRED));
		shader->Bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->ColorAttachment());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->ColorAttachment(1));
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->ColorAttachment(2));



		GLint i = 0;
		for (auto& light : lightView)
		{
			const auto position = lightView.get<ECS::TransformComponent>(light).Translation;
			const ECS::LightComponent& lightCmp = lightView.get<ECS::LightComponent>(light);

			shader->SetUniformRef(g_UniformLocations[i].Position, position);
			shader->SetUniformRef(g_UniformLocations[i].Color, lightCmp.Color);
			// update attenuation parameters and calculate radius
			constexpr float constant = 1.0f; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
			constexpr float linear = 0.7f;
			constexpr float quadratic = 1.8f;
			shader->SetUniform(g_UniformLocations[i].Linear, linear);
			shader->SetUniform(g_UniformLocations[i].Quadratic, quadratic);
			// then calculate radius of light volume/sphere
			const float maxBrightness = std::fmaxf(std::fmaxf(lightCmp.Color.r, lightCmp.Color.g), lightCmp.Color.b);
			const float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
			shader->SetUniform(g_UniformLocations[i].Radius, radius);

			i++;
		}
		g_LightCount = i;
		shader->SetUniform(g_LightNumLocation, g_LightCount);
		shader->SetUniformRef(g_ViewPosLocation, camera->GetPosition());

		glDrawArrays(GL_TRIANGLES, 0, 6);

		shader->UnBind();
		buffer.UnBind();
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
}
