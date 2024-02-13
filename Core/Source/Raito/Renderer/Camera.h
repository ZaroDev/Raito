#pragma once

#include <Math/MathTypes.h>

namespace Raito::Renderer
{
	class Camera
	{
	public:
		Camera(float verticalFOV, float nearClip, float farClip);

		bool OnUpdate(float ts);
		void OnResize(u32 width, u32 height);

		const Mat4& GetProjection() const { return m_Projection; }
		const Mat4& GetInverseProjection() const { return m_InverseProjection; }
		const Mat4& GetView() const { return m_View; }
		const Mat4& GetInverseView() const { return m_InverseView; }

		const V3& GetPosition() const { return m_Position; }
		const V3& GetDirection() const { return m_ForwardDirection; }


		float GetRotationSpeed();
	private:
		void RecalculateProjection();
		void RecalculateView();

	private:
		Mat4 m_Projection{ 1.0f };
		Mat4 m_View{ 1.0f };
		Mat4 m_InverseProjection{ 1.0f };
		Mat4 m_InverseView{ 1.0f };

		float m_VerticalFOV = 45.0f;
		float m_NearClip = 0.1f;
		float m_FarClip = 100.0f;

		V3 m_Position{ 0.0f, 0.0f, 0.0f };
		V3 m_ForwardDirection{ 0.0f, 0.0f, 0.0f };

		V2 m_LastMousePosition{ 0.0f, 0.0f };

		u32 m_ViewportWidth = 0, m_ViewportHeight = 0;
	};
}