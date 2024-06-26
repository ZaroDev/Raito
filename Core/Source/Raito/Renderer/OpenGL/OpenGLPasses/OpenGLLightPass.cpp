#include <pch.h>
#include "OpenGLLightPass.h"

#include "Core/Application.h"
#include "ECS/Components.h"


namespace Raito::Renderer::OpenGL::LightPass
{
	namespace
	{
		constexpr u32 c_MaxLights = 1024;
		constexpr u32 c_DirectionalSize = sizeof(Mat3);
		constexpr u32 c_PointSize = sizeof(u32) + sizeof(Mat3) * c_MaxLights;
		u32 g_DirectionalSSBO;
		u32 g_PointSSBO;

		std::vector<Mat4> g_Points;
	}

	bool Initialize()
	{
		glGenBuffers(1, &g_DirectionalSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_DirectionalSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, c_DirectionalSize, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glGenBuffers(1, &g_PointSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_PointSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, c_PointSize, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		return true;
	}

	void Update(const Camera& camera)
	{
		const auto& scene = Core::Application::Get().Scene;
		const auto view = scene.GetAllEntitiesWith<ECS::TransformComponent, ECS::LightComponent>();

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_DirectionalSSBO);

		u32 pointSize = 0;

		g_Points.clear();

		for (const auto& entity : view)
		{
			const auto& light = view.get<ECS::LightComponent>(entity);
			const auto& transform = view.get<ECS::TransformComponent>(entity);

			switch (light.LightType)
			{
			case ECS::LightComponent::Type::DIRECTIONAL:
			{
				const auto dir = Mat3(light.Direction, light.Color, V3{0.0f});
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Mat3) , &dir);
			}break;
			case ECS::LightComponent::Type::POINT_LIGHT:
			{
				pointSize++;
				const auto point = Mat3(transform.Translation, light.Color, V3{0.f});
				g_Points.emplace_back(point);

			}break;
			case ECS::LightComponent::Type::SPOT_LIGHT:
			{
			}
			break;
			}
		}




		glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_PointSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Mat3) * g_Points.size(), g_Points.data());
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	u32 GetDirectionalSSBO()
	{
		return g_DirectionalSSBO;
	}

	u32 GetPointSSBO()
	{
		return g_PointSSBO;
	}

	u32 GetPointSize()
	{
		return g_Points.size();
	}

	void Shutdown()
	{
		glDeleteBuffers(1, &g_DirectionalSSBO);
		g_Points.clear();
	}
}
