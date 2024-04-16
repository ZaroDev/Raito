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

		const unsigned int NR_LIGHTS = 32;
		std::vector<V3> lightPositions;
		std::vector<V3> lightColors;
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


		for (unsigned int i = 0; i < NR_LIGHTS; i++)
		{
			// calculate slightly random offsets
			float xPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
			float yPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 4.0);
			float zPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
			lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
			// also calculate random color
			float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
			float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
			float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
			lightColors.push_back(glm::vec3(rColor, gColor, bColor));
		}

		const auto shader = dynamic_cast<OpenGL::OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::DEFERRED));
		shader->Bind();
		shader->SetUniform("gPosition", 0);
		shader->SetUniform("gNormal", 1);
		shader->SetUniform("gAlbedoSpec", 2);


		return true;
	}

	void Update(Camera* camera, const OpenGLFrameBuffer& buffer)
	{
		buffer.Bind();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
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
		
		
		glBindVertexArray(g_FrameBufferQuadVAO);
		glDisable(GL_DEPTH_TEST);

		const auto shader = dynamic_cast<OpenGL::OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::DEFERRED));
		shader->Bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, buffer.ColorAttachment());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, buffer.ColorAttachment(1));
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, buffer.ColorAttachment(2));


		for (unsigned int i = 0; i < lightPositions.size(); i++)
		{
			shader->SetUniformRef(std::string("lights[" + std::to_string(i) + "].Position").c_str(), lightPositions[i]);
			shader->SetUniformRef(std::string("lights[" + std::to_string(i) + "].Color").c_str(), lightColors[i]);
			// update attenuation parameters and calculate radius
			const float constant = 1.0f; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
			const float linear = 0.7f;
			const float quadratic = 1.8f;
			shader->SetUniform(std::string("lights[" + std::to_string(i) + "].Linear").c_str(), linear);
			shader->SetUniform(std::string("lights[" + std::to_string(i) + "].Quadratic").c_str(), quadratic);
			// then calculate radius of light volume/sphere
			const float maxBrightness = std::fmaxf(std::fmaxf(lightColors[i].r, lightColors[i].g), lightColors[i].b);
			float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
			shader->SetUniform(std::string("lights[" + std::to_string(i) + "].Radius").c_str(), radius);
		}

		shader->SetUniformRef("u_ViewPos", camera->GetPosition());

		glDrawArrays(GL_TRIANGLES, 0, 6);

		shader->UnBind();
		buffer.UnBind();

	}

	void Shutdown()
	{
		glDeleteVertexArrays(1, &g_FrameBufferQuadVAO);
		glDeleteBuffers(1, &g_FrameBufferQuadVBO);
	}
}
