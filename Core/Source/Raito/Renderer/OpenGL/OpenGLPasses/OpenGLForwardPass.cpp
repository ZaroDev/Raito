#include <pch.h>
#include "OpenGLForwardPass.h"

#include "OpenGLLightPass.h"
#include "OpenGLShadowPass.h"
#include "OpenGLSkyboxPass.h"
#include "Core/Application.h"
#include "ECS/Components.h"
#include "Renderer/Camera.h"
#include "Renderer/OpenGL/OpenGLCore.h"
#include "Renderer/OpenGL/OpenGLObjects/OpenGLShader.h"
#include "OpenGLSSAOPass.h"

namespace Raito::Renderer::OpenGL::Forward
{
	namespace
	{
		
	}

	bool Initialize()
	{
		const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(FORWARD));
		shader->Bind();

		shader->SetUniform("u_ShadowMap", 0);
		shader->SetUniform("u_IrradianceMap", 1);
		shader->SetUniform("u_PrefilterMap", 2);


		shader->UnBind();
		return true;
	}

	void Update(const Camera& camera, const OpenGLFrameBuffer& buffer)
	{
		buffer.Bind();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(FORWARD));
		shader->Bind();
		// Geometry pass
		auto& scene = Core::Application::Get().Scene;
		const auto view = scene.GetAllEntitiesWith<ECS::TransformComponent, ECS::MeshComponent>();
		for (auto& entity : view)
		{
			const ECS::MeshComponent& mesh = view.get<ECS::MeshComponent>(entity);
			const OpenGLMeshData& meshData = GetMesh(mesh.MeshId);
			const Mat4 model = view.get<ECS::TransformComponent>(entity).GetTransform();
			const Mat3 normalMatrix = glm::transpose(glm::inverse(Mat3(model)));

			const Assets::PbrMaterial& mat = GetMaterial(mesh.MaterialId);

			SetTextureOnShader("u_Albedo", *shader, mat.Albedo);
			SetTextureOnShader("u_Normal", *shader, mat.Normal);
			SetTextureOnShader("u_Emissive", *shader, mat.Emissive);
			SetTextureOnShader("u_MetalRoughness", *shader, mat.MetalRoughness);
			SetTextureOnShader("u_SSAO", *shader, { SSAO::GetSSAOHandle(), 0 });
			SetTextureOnShader("u_BRDFLUT", *shader,  { Skybox::GetBRDFLUTTMap(), 0 });


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


			shader->SetUniformRef("u_View", camera.GetView());
			shader->SetUniformRef("u_Projection", camera.GetProjection());
			shader->SetUniformRef("u_Model", model);
			shader->SetUniformRef("u_NormalMatrix", normalMatrix);
			shader->SetUniform("u_PointSize", LightPass::GetPointSize());
			shader->SetUniformRef("u_ViewPos", camera.GetPosition());

			glBindVertexArray(meshData.VAO);
			glDrawElements(meshData.RenderMode, meshData.IndexCount, GL_UNSIGNED_INT, nullptr);
			glBindVertexArray(0);
		}
		shader->UnBind();

		Skybox::Update(camera);
		buffer.UnBind();
	}

	void Shutdown()
	{


	}
}
