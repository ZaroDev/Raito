#pragma once

#include <Renderer/OpenGL/OpenGLCommon.h>
#include <Renderer/OpenGL/OpenGLObjects/OpenGLShader.h>

namespace Raito::Renderer::OpenGL
{
	struct UniformValue
	{
		Uniform Data;
		ubyte* Value;
	};

	class OpenGLMaterial final
	{
	public:
		explicit OpenGLMaterial(u32 shaderId);
		~OpenGLMaterial();

		void Use();

		template<typename T>
		void SetValue(const char* name, const T value)
		{
			m_Uniforms[name].Value = static_cast<ubyte*>(value);
		}

	private:
		u32 m_ShaderId;

		std::unordered_map<std::string, UniformValue> m_Uniforms{};
	};
}