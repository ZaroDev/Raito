#pragma once

#include <Renderer/OpenGL/OpenGLCommon.h>

namespace Raito::Renderer::OpenGL
{
	class OpenGLMaterial
	{
	public:
		OpenGLMaterial(u32 shaderId);
		~OpenGLMaterial();

		void Bind();
		void UnBind();



	private:
		u32 m_ShaderId;

	};
}