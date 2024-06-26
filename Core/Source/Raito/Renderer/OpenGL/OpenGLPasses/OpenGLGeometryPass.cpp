#include <pch.h>
#include "OpenGLGeometryPass.h"

#include "ECS/Components.h"
#include "Renderer/OpenGL/OpenGLCore.h"
#include "Renderer/OpenGL/OpenGLObjects/OpenGLMaterial.h"

namespace Raito::Renderer::OpenGL::Geometry
{
	namespace
	{
		bool g_DebugAABB = false;
		bool g_EnableCulling = true;
	}

	void EnableAABBDebug(bool value)
	{
		g_DebugAABB = value;
	}

	void EnableCulling(bool value)
	{
		g_EnableCulling = value;
	}

	void Update(const ECS::Scene& scene, const Camera& camera, const OpenGLFrameBuffer& buffer, OpenGLShader* shader)
	{
		const auto view = scene.GetAllEntitiesWith<ECS::TransformComponent, ECS::MeshComponent>();
		for (auto& entity : view)
		{
			const ECS::MeshComponent& mesh = view.get<ECS::MeshComponent>(entity);
			const OpenGLMeshData& meshData = GetMesh(mesh.MeshId);
			const auto& transform = view.get<ECS::TransformComponent>(entity);
			const Mat4 model = transform.GetTransform();
			const V3 location = transform.Translation;
			const Mat3 normalMatrix = glm::transpose(glm::inverse(Mat3(model)));

			// Construct a AABB with the model translation
			auto aabb = Math::AABB(meshData.AABB.GetMin(), meshData.AABB.GetMax());
			aabb.Translate(location);
			aabb.Scale(transform.Scale);
			if (g_EnableCulling)
			{
				if (!camera.IsInsideFrustum(aabb))
				{
					continue;
				}
			}


			shader->Bind();

			Assets::PbrMaterial mat = GetMaterial(mesh.MaterialId);

			SetTextureOnShader("u_Albedo", *shader, mat.Albedo);
			SetTextureOnShader("u_Normal", *shader, mat.Normal);
			SetTextureOnShader("u_Emissive", *shader, mat.Emissive);
			SetTextureOnShader("u_MetalRoughness", *shader, mat.MetalRoughness);
			SetTextureOnShader("u_AmbientOcclusion", *shader, mat.AmbientOcclusion);

			shader->SetUniformRef("u_View", camera.GetView());
			shader->SetUniformRef("u_Projection", camera.GetProjection());
			shader->SetUniformRef("u_Model", model);
			shader->SetUniformRef("u_NormalMatrix", normalMatrix);

			glBindVertexArray(meshData.VAO);
			glDrawElements(meshData.RenderMode, meshData.IndexCount, GL_UNSIGNED_INT, nullptr);
			glBindVertexArray(0);

			shader->UnBind();

			if(g_DebugAABB)
			{
				
			}
		}
	}

	void DrawDebug(const Math::AABB& aabb, const Camera& camera)
	{
		const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(FORWARD));
		shader->Bind();
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		Mat4 aabbModel = Mat4(1.0);
		aabbModel = glm::translate(aabbModel, aabb.GetCenter());
		aabbModel = glm::scale(aabbModel, aabb.GetExtent());

		shader->SetUniformRef("u_Model", aabbModel);
		shader->SetUniformRef("u_View", camera.GetView());
		shader->SetUniformRef("u_Projection", camera.GetProjection());

		RenderCube();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		shader->UnBind();
	}
}
