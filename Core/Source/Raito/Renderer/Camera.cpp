/*
MIT License

Copyright (c) 2023 Víctor Falcón Zaro

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <pch.h>
#include "Camera.h"

#include "Input/Input.h"
#include "optick/src/optick.h"


namespace Raito::Renderer
{
	Camera::Camera(float verticalFOV, float nearClip, float farClip)
		: m_VerticalFOV(verticalFOV), m_NearClip(nearClip), m_FarClip(farClip)
	{
		m_ForwardDirection = glm::vec3(0, 0, -1);
		m_Position = glm::vec3(0, 0, 5);
		RecalculateView();

		m_Frustum = Math::Frustum(m_Projection * m_View);
	}

	bool Camera::OnUpdate(float ts)
	{
		OPTICK_CATEGORY("Update Camera", Optick::Category::Camera);
		bool moved = false;
		
		V2 mousePos = Input::GetMousePosition();
		V2 delta = (mousePos - m_LastMousePosition) * 0.002f;
		m_LastMousePosition = mousePos;

		if (!Input::IsMouseButtonDown(MouseButton::Right))
		{
			Input::SetCursorMode(CursorMode::Normal);
			return false;
		}

		Input::SetCursorMode(CursorMode::Locked);


		constexpr V3 upDirection(0.0f, 1.0f, 0.0f);
		V3 rightDirection = glm::cross(m_ForwardDirection, upDirection);

		float speed = 5.0f;

		// Movement
		if (Input::IsKeyDown(KeyCode::W))
		{
			m_Position += m_ForwardDirection * speed * ts;
			moved = true;
		}
		else if (Input::IsKeyDown(KeyCode::S))
		{
			m_Position -= m_ForwardDirection * speed * ts;
			moved = true;
		}
		if (Input::IsKeyDown(KeyCode::A))
		{
			m_Position -= rightDirection * speed * ts;
			moved = true;
		}
		else if (Input::IsKeyDown(KeyCode::D))
		{
			m_Position += rightDirection * speed * ts;
			moved = true;
		}
		if (Input::IsKeyDown(KeyCode::Q))
		{
			m_Position -= upDirection * speed * ts;
			moved = true;
		}
		else if (Input::IsKeyDown(KeyCode::E))
		{
			m_Position += upDirection * speed * ts;
			moved = true;
		}

		// Rotation
		if (delta.x != 0.0f || delta.y != 0.0f)
		{
			float pitchDelta = delta.y * GetRotationSpeed();
			float yawDelta = delta.x * GetRotationSpeed();

			Quaternion q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightDirection),
				glm::angleAxis(-yawDelta, V3(0.f, 1.0f, 0.0f))));
			m_ForwardDirection = glm::rotate(q, m_ForwardDirection);

			moved = true;
		}

		if (moved)
		{
			RecalculateView();
		}

		
		return moved;
	}

	void Camera::OnResize(uint32_t width, uint32_t height)
	{
		if (width == m_ViewportWidth && height == m_ViewportHeight)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		const float ar = m_ViewportWidth / m_ViewportHeight;

		if (ar >= 1.0)
		{
			m_Parameters.Left = -ar * m_ViewportWidth;
			m_Parameters.Right = ar * m_ViewportWidth;
			m_Parameters.Top = m_ViewportWidth;
			m_Parameters.Bottom = -m_ViewportWidth;
		}
		else
		{
			m_Parameters.Left = -m_ViewportWidth;
			m_Parameters.Right = m_ViewportWidth;
			m_Parameters.Top =  m_ViewportWidth / ar;
			m_Parameters.Bottom = -m_ViewportWidth / ar;
		}

		RecalculateProjection();
	}

	float Camera::GetRotationSpeed()
	{
		return 0.3f;
	}

	bool Camera::IsInsideFrustum(const Math::AABB& boundingBox) const
	{
		if(!FrustumCulling)
		{
			return true;
		}

		return m_Frustum.IsBoxVisible(boundingBox.GetMin(), boundingBox.GetMax());
	}

	void Camera::RecalculateProjection()
	{
		m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFOV), (float)m_ViewportWidth, (float)m_ViewportHeight, m_NearClip, m_FarClip);
		m_InverseProjection = glm::inverse(m_Projection);
		m_Frustum = Math::Frustum(m_Projection * m_View);
	}

	void Camera::RecalculateView()
	{
		m_View = glm::lookAt(m_Position, m_Position + m_ForwardDirection, glm::vec3(0, 1, 0));
		m_InverseView = glm::inverse(m_View);
		m_Frustum = Math::Frustum(m_Projection * m_View);
	}

}
