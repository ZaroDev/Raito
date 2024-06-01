#include <pch.h>
#include "OpenGLShadowPass.h"
#include <Raito/Renderer/Camera.h>
#include <Raito/Renderer/OpenGL/OpenGLCore.h>
#include "Core/Application.h"
#include "ECS/Components.h"
#include "Window/Window.h"
namespace Raito::Renderer::OpenGL::Shadows
{
	namespace
	{
		std::vector<float> g_ShadowCascadeLevels{};

		u32 g_MatricesUBO;
		u32 g_LightFBO;

		std::vector<V4> GetFrustumCornersWorldSpace(const Mat4& proj, const Mat4& view)
		{
			const auto inv = glm::inverse(proj * view);

			std::vector<V4> frustumCorners;
			for (unsigned int x = 0; x < 2; ++x)
			{
				for (unsigned int y = 0; y < 2; ++y)
				{
					for (unsigned int z = 0; z < 2; ++z)
					{
						const V4 pt =
							inv * V4(
								2.0f * x - 1.0f,
								2.0f * y - 1.0f,
								2.0f * z - 1.0f,
								1.0f);
						frustumCorners.push_back(pt / pt.w);
					}
				}
			}

			return frustumCorners;
		}

		Mat4 GetLightSpaceMatrix(const float nearPlane, const float farPlane, const V3& lightDir)
		{
			const Camera& camera = GetMainCamera();
			const SysWindow& window = Window::GetWindow();
			const auto proj = glm::perspective(
				glm::radians(camera.GetFOV()),
				static_cast<float>(window.Info.Width) / static_cast<float>(window.Info.Height),
				nearPlane,
				farPlane
			);

			const auto corners = GetFrustumCornersWorldSpace(proj, camera.GetView());
			auto center = V3(0.0f);
			for (const auto& v : corners)
			{
				center += V3(v);
			}
			center /= corners.size();

			const auto lightView = glm::lookAt(center + lightDir, center, V3(0.0f, 1.0f, 0.0f));

			float minX = std::numeric_limits<float>::max();
			float maxX = std::numeric_limits<float>::lowest();
			float minY = std::numeric_limits<float>::max();
			float maxY = std::numeric_limits<float>::lowest();
			float minZ = std::numeric_limits<float>::max();
			float maxZ = std::numeric_limits<float>::lowest();

			for (const auto& v : corners)
			{
				const auto trf = lightView * v;
				minX = std::min(minX, trf.x);
				maxX = std::max(maxX, trf.x);
				minY = std::min(minY, trf.y);
				maxY = std::max(maxY, trf.y);
				minZ = std::min(minZ, trf.z);
				maxZ = std::max(maxZ, trf.z);
			}
			// Tune this parameter according to the scene
			constexpr float zMult = 5.0f;
			if (minZ < 0)
			{
				minZ *= zMult;
			}
			else
			{
				minZ /= zMult;
			}
			if (maxZ < 0)
			{
				maxZ /= zMult;
			}
			else
			{
				maxZ *= zMult;
			}

			const Mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
			return lightProjection * lightView;
		}

		std::vector<Mat4> GetLightSpaceMatrices(const V3& lightDir, const Camera& camera)
		{
			g_ShadowCascadeLevels =
			{
				camera.GetNearFar() / 50.0f,
				camera.GetNearFar() / 25.0f,
				camera.GetNearFar() / 10.0f,
				camera.GetNearFar() / 2.0f
			};
			std::vector<Mat4> result;
			for (u32 i = 0; i < g_ShadowCascadeLevels.size() + 1; i++)
			{
				if (i == 0)
				{
					result.emplace_back(GetLightSpaceMatrix(camera.GetNearClip(), g_ShadowCascadeLevels[i], lightDir));
				}
				else if(i < g_ShadowCascadeLevels.size())
				{
					result.emplace_back(GetLightSpaceMatrix(g_ShadowCascadeLevels[i - 1], g_ShadowCascadeLevels[i], lightDir));
				}
				else
				{
					result.emplace_back(GetLightSpaceMatrix(g_ShadowCascadeLevels[i - 1], camera.GetNearFar(), lightDir));
				}
			}

			return result;
		}

		void GenerateMatricesUBO()
		{
			glGenBuffers(1, &g_MatricesUBO);
			glBindBuffer(GL_UNIFORM_BUFFER, g_MatricesUBO);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(Mat4) * 16, nullptr, GL_STATIC_DRAW);
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_MatricesUBO);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		u32 g_LightDepthMaps;
		constexpr u32 c_DepthMapResolution = 4096;
		void GenerateLightFBO()
		{
			glGenFramebuffers(1, &g_LightFBO);
			glGenTextures(1, &g_LightDepthMaps);
			glBindTexture(GL_TEXTURE_2D_ARRAY, g_LightDepthMaps);
			glTexImage3D(
				GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32,
				c_DepthMapResolution, c_DepthMapResolution, static_cast<int>(g_ShadowCascadeLevels.size()) + 1,
				0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr
			);

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			
			constexpr float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

			glBindFramebuffer(GL_FRAMEBUFFER, g_LightFBO);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, g_LightDepthMaps, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				O_ERROR("Shadow map framebuffer is not compleated");
			}


			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}


	bool Initialize()
	{
		const float farPlane = GetMainCamera().GetNearFar();
		g_ShadowCascadeLevels =
		{
			farPlane / 50.0f,
			farPlane / 25.0f,
			farPlane / 10.0f,
			farPlane / 2.0f
		};
		GenerateMatricesUBO();
		GenerateLightFBO();

		return true;
	}

	void Update(Camera* camera)
	{
	
		auto& scene = Core::Application::Get().Scene;
		{
			const auto lightView = scene.GetAllEntitiesWith<ECS::LightComponent>();
			auto directionalLight = V3(0.0);
			for (auto& light : lightView)
			{
				const ECS::LightComponent& lightCmp = lightView.get<ECS::LightComponent>(light);
				if (lightCmp.LightType == ECS::LightComponent::Type::DIRECTIONAL)
				{
					directionalLight = glm::normalize(lightCmp.Direction);
				}
			}

			const auto lightMatrices = GetLightSpaceMatrices(directionalLight, *camera);
			glBindBuffer(GL_UNIFORM_BUFFER, g_MatricesUBO);
			for (u32 i = 0; i < lightMatrices.size(); i++)
			{
				glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(Mat4), sizeof(Mat4), &lightMatrices[i]);
			}
		}
		{
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DEPTH));
			shader->Bind();

			glBindFramebuffer(GL_FRAMEBUFFER, g_LightFBO);

			glViewport(0, 0, c_DepthMapResolution, c_DepthMapResolution);
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);
			glClear(GL_DEPTH_BUFFER_BIT);
			glCullFace(GL_FRONT);

			const auto view = scene.GetAllEntitiesWith<ECS::TransformComponent, ECS::MeshComponent>();

			for (auto& entity : view)
			{
				const ECS::MeshComponent& mesh = view.get<ECS::MeshComponent>(entity);
				const OpenGLMeshData& meshData = GetMesh(mesh.MeshId);
				const auto& transform = view.get<ECS::TransformComponent>(entity);
				const Mat4 model = transform.GetTransform();

				shader->SetUniformRef("u_Model", model);

				glBindVertexArray(meshData.VAO);
				glDrawElements(meshData.RenderMode, meshData.IndexCount, GL_UNSIGNED_INT, nullptr);
				glBindVertexArray(0);
			}
			glCullFace(GL_BACK);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
			shader->UnBind();
		}
	}

	void Shutdown()
	{

	}

	u32 GetLightSpaceMatricesUBO()
	{
		return g_MatricesUBO;
	}

	u32 GetShadowMap()
	{
		return g_LightDepthMaps;
	}

	const std::vector<float>& GetCascadeLevels()
	{
		return g_ShadowCascadeLevels;
	}
}
