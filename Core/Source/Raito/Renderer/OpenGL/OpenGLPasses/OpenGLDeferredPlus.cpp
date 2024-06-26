#include <pch.h>
#include "OpenGLDeferredPlus.h"

#include "OpenGLGeometryPass.h"
#include "Core/Application.h"
#include "ECS/Components.h"
#include "Renderer/OpenGL/OpenGLCore.h"
#include "Renderer/OpenGL/OpenGLObjects/OpenGLShader.h"

namespace Raito::Renderer::OpenGL::DeferredPlus
{
	namespace
	{
		constexpr u32 g_MaxObjectBufferSize = 1000;
		std::unique_ptr<OpenGLFrameBuffer> g_FrameBuffer = nullptr;
		std::unique_ptr<OpenGLFrameBuffer> g_VisibilityBuffer = nullptr;

		struct VisibilityBuffers
		{
			GLuint BoundingBoxModel;
			GLuint Visibility;
			GLuint DrawCommands;
			GLuint MeshInfo;
			GLuint OccludeInstanceIndex;
			GLuint VisibleInstanceIndex;
		} g_VisibilityBuffers;


		struct DrawCommand
		{
			u32 Count;
			u32 InstanceCount;
			u32 FirstIndex;
			i32 BaseVertex;
			u32 BaseInstance;
		};

		struct MeshInfo
		{
			u32 MaterialID;
			u32 NumInstances;
			u32 InstanceOffset;
			u32 MeshType;
			u32 MeshTypeOffset;
			u32 NumIndices;
			u32 FirstIndex;
		};


		
		u32 g_DownSampledDepth;
		u64 g_DownSampledHandle;

		void GenerateBuffer(u32& id, GLenum type,size_t size, GLenum flags)
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

		g_VisibilityBuffer = std::make_unique<OpenGLFrameBuffer>(
			FrameBufferData{
				{
					FrameBufferTextureFormat::Depth		// Depth buffer
				},
				1920 / 4,
				1080 / 4,
				1
			});


		glGenTextures(1, &g_DownSampledDepth);
		glBindTexture(GL_TEXTURE_2D, g_DownSampledDepth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1920 / 4, 1080 / 4, 0, GL_RED, GL_FLOAT, NULL);
		glBindImageTexture(1, g_DownSampledDepth, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

		g_DownSampledHandle = glGetTextureHandleARB(g_DownSampledDepth);
		glMakeTextureHandleResidentARB(g_DownSampledHandle);

		GenerateBuffer(g_VisibilityBuffers.BoundingBoxModel, GL_SHADER_STORAGE_BUFFER, sizeof(Mat4) * g_MaxObjectBufferSize, GL_DYNAMIC_DRAW);
		GenerateBuffer(g_VisibilityBuffers.Visibility, GL_SHADER_STORAGE_BUFFER, sizeof(u32) * g_MaxObjectBufferSize, GL_DYNAMIC_DRAW);
		GenerateBuffer(g_VisibilityBuffers.DrawCommands, GL_DRAW_INDIRECT_BUFFER, sizeof(DrawCommand) * g_MaxObjectBufferSize, GL_DYNAMIC_DRAW);
		GenerateBuffer(g_VisibilityBuffers.MeshInfo, GL_SHADER_STORAGE_BUFFER, sizeof(MeshInfo) * g_MaxObjectBufferSize, GL_DYNAMIC_DRAW);
		GenerateBuffer(g_VisibilityBuffers.VisibleInstanceIndex, GL_SHADER_STORAGE_BUFFER, sizeof(u32) * g_MaxObjectBufferSize, GL_DYNAMIC_DRAW);
		GenerateBuffer(g_VisibilityBuffers.OccludeInstanceIndex, GL_SHADER_STORAGE_BUFFER, sizeof(u32) * g_MaxObjectBufferSize, GL_DYNAMIC_DRAW);

		{
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DRAW_COMMAND));
			shader->Bind();
			const uint32_t storageBlockIndex = glGetProgramResourceIndex(shader->Id(), GL_SHADER_STORAGE_BLOCK, "DrawCommandsBlock");
			glShaderStorageBlockBinding(shader->Id(), storageBlockIndex, 1);

			shader->UnBind();
		}


		return true;
	}

	void Update(const Camera& camera, const OpenGLFrameBuffer& buffer)
	{
		const auto& scene = Core::Application::Get().Scene;

		// Depth buffer down-sampling and re-projection
		{
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DOWN_SAMPLE_REPROJECT));
			shader->Bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->DepthAttachment());

			shader->SetUniformRef("u_Projection", camera.GetProjection());
			shader->SetUniformRef("u_View", camera.GetView());
			glDispatchCompute(g_FrameBuffer->Data().Width, g_FrameBuffer->Data().Height, 1);

			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			shader->UnBind();
		}
		// Visibility buffer
		{
			g_VisibilityBuffer->Bind();

			// Copy depth buffer
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, 1920 / 4, 1080 / 4);
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_ALWAYS);
				glDepthMask(GL_TRUE);
				const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(COPY_DEPTH));
				shader->Bind();

				glUniformHandleui64ARB(shader->GetUniformLocation("u_ScreenTexture"), g_DownSampledHandle);

				RenderFullScreenQuad();
				shader->UnBind();

				glDepthFunc(GL_LESS);
			}

			// Clear visibility buffer
			{
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_VisibilityBuffers.Visibility);
				constexpr u32 value = 0;
				glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32I, GL_RED_INTEGER, GL_UNSIGNED_INT, &value);
			}

			u32 objectCount = 0;
			std::vector<OpenGLMeshData> visibleMesh;
			// Render AABB
			{
				glDisable(GL_DEPTH_TEST);
				glDepthMask(GL_FALSE);
				const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(VISIBILITY_BUFFER));
				shader->Bind();

				const auto view = scene.GetAllEntitiesWith<ECS::TransformComponent, ECS::MeshComponent>();
				std::vector<Mat4> matrices;
				std::vector<MeshInfo> meshInfos;
				for (const auto& entity : view)
				{
					const auto& transform = view.get<ECS::TransformComponent>(entity);
					const ECS::MeshComponent& mesh = view.get<ECS::MeshComponent>(entity);
					const OpenGLMeshData& meshData = GetMesh(mesh.MeshId);
					auto aabb = Math::AABB(meshData.AABB.GetMin(), meshData.AABB.GetMax());
					aabb.Translate(transform.Translation);
					aabb.Scale(transform.Scale);

					Mat4 aabbModel = Mat4(1.0);
					aabbModel = translate(aabbModel, aabb.GetCenter());
					aabbModel = scale(aabbModel, aabb.GetExtent());

					MeshInfo info{};
					info.MaterialID = mesh.MaterialId;
					info.NumInstances = 1;
					info.FirstIndex = 0;
					info.InstanceOffset = 0;
					info.MeshType = 0;
					info.NumIndices = meshData.IndexCount;
					info.MeshTypeOffset = 0;
					meshInfos.emplace_back(info);

					if (!camera.IsInsideFrustum(aabb))
					{
						continue;
					}

					objectCount++;
					matrices.emplace_back(aabbModel);
					visibleMesh.emplace_back(meshData);
				}
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_VisibilityBuffers.MeshInfo);
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(MeshInfo) * meshInfos.size(), meshInfos.data());

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_VisibilityBuffers.BoundingBoxModel);
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Mat4) * matrices.size(), matrices.data());
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_VisibilityBuffers.BoundingBoxModel);


				glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_VisibilityBuffers.Visibility);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_VisibilityBuffers.Visibility);

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

				shader->SetUniformRef("u_Projection", camera.GetProjection());
				shader->SetUniformRef("u_View", camera.GetView());

				RenderCubeInstanced(matrices.size());

				shader->UnBind();

				g_VisibilityBuffer->UnBind();
				glDepthMask(GL_TRUE);
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LEQUAL);
			}

			// Generate indirect draw information
			{
				const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DRAW_COMMAND));
				shader->Bind();
				

				// Visibility buffer
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_VisibilityBuffers.Visibility);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_VisibilityBuffers.Visibility);

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_VisibilityBuffers.DrawCommands);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_VisibilityBuffers.DrawCommands);

				// Mesh info buffer
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_VisibilityBuffers.MeshInfo);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, g_VisibilityBuffers.MeshInfo);

				// Occluded instance buffer
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_VisibilityBuffers.OccludeInstanceIndex);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, g_VisibilityBuffers.OccludeInstanceIndex);

				// Visible instance buffer
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_VisibilityBuffers.VisibleInstanceIndex);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, g_VisibilityBuffers.VisibleInstanceIndex);

				glDispatchCompute(objectCount, 1, 1);
				glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
				shader->UnBind();
			}

			// Render indirectly visible meshes
			{
				for (const auto& mesh : visibleMesh)
				{
					glBindVertexArray(mesh.VAO);
					glBindBuffer(GL_DRAW_INDIRECT_BUFFER, g_VisibilityBuffers.DrawCommands);
					glDrawElementsIndirect(mesh.RenderMode, GL_UNSIGNED_INT, nullptr);
					glBindVertexArray(0);
				}
			}
		}

		g_FrameBuffer->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, 1920, 1080);

		g_FrameBuffer->UnBind();
	}

	u32 GetDeferredAttachment(u32 id)
	{
		return g_FrameBuffer->ColorAttachment(id);
	}

	u32 GetDeferredDepth()
	{
		return g_VisibilityBuffer->DepthAttachment();
	}

	void Shutdown()
	{
		g_FrameBuffer.release();
		g_VisibilityBuffer.release();
	}
}
