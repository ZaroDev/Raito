#include <pch.h>
#include "OpenGLForwardPass.h"

#include "Core/Application.h"
#include "ECS/Components.h"
#include "Renderer/Camera.h"
#include "Renderer/OpenGL/OpenGLObjects/OpenGLMaterial.h"
#include "Renderer/OpenGL/OpenGLObjects/OpenGLShader.h"

namespace Raito::Renderer::OpenGL::Forward
{

	bool Initialize()
	{
		return true;
	}

	void Update(Camera* camera, const OpenGLFrameBuffer& buffer)
	{
		buffer.Bind();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		// Geometry pass

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


			material.SetValue("u_ViewPos", camera->GetPosition());
			for (auto& light : lightView)
			{
				material.SetValue("u_LightPosition", lightView.get<ECS::TransformComponent>(light).Translation);
				material.SetValue("u_LightColor", lightView.get<ECS::LightComponent>(light).Color);
			}

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
		buffer.UnBind();
	}

	void Shutdown()
	{


	}
}
