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