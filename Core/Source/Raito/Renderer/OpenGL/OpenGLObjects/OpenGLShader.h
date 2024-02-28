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

#include <Renderer/OpenGL/OpenGLCommon.h>
#include <Renderer/OpenGL/OpenGLShaderCompiler.h>
#include <Renderer/Shader.h>

#include <Math/MathTypes.h>
#include <glm/gtc/type_ptr.hpp>

namespace Raito::Renderer::OpenGL
{
	enum OpenGLShaderType : u32
	{
		VERTEX = (1u << 0),
		TESSELATION = (1u << 1),
		GEOMETRY = (1u << 2),
		FRAGMENT = (1u << 3),
		COMPUTE = (1u << 4),

		MAX = (1u << 5)
	};

	enum class UniformType : u16
	{
		NONE = 0,
		INT,
		FLOAT,
		VEC3,
		VEC4,
		MAT3,
		MAT4,
		SAMPLER_2D
	};

	struct Uniform
	{
		GLint Id = 0;
		UniformType Type = UniformType::NONE;
	};

	class OpenGLShader final : public Shader
	{
	public:
		OpenGLShader(u32 id);
		virtual ~OpenGLShader();


		DEFAULT_MOVE_AND_COPY(OpenGLShader)

		void Bind() const;
		void UnBind() const;


		template<typename T>
		void SetUniform(u32 id, const T value)
		{
			ASSERT(false);
		}
		template<typename T>
		void SetUniformRef(u32 id, const T& value)
		{
			ASSERT(false);
		}

		template<typename T>
		void SetUniform(const char* uniformName, const T value)
		{
			SetUniform<T>(GetUniformLocation(uniformName), value);
		}
		template<typename T>
		void SetUniformRef(const char* uniformName, const T& value)
		{
			SetUniformRef<T>(GetUniformLocation(uniformName), value);
		}

		NODISCARD OpenGLShaderType ShaderType() const { return m_ShaderType; }
		NODISCARD size_t UniformCount() const { return m_Uniforms.size(); }
		NODISCARD const std::unordered_map<std::string, Uniform>& Uniforms() const { return m_Uniforms; }
		NODISCARD u32 Id() const { return m_ShaderId; }


		GLint GetUniformLocation(const char* uniformName) const
		{
			if(m_Uniforms.contains(uniformName))
			{
				return m_Uniforms.at(uniformName).Id;
			}

			return 0;
		}

	private:
		u32 m_ShaderId = 0;
		OpenGLShaderType m_ShaderType = OpenGLShaderType::MAX;

		std::unordered_map<std::string, Uniform> m_Uniforms{};

		friend u32 ShaderCompiler::CompileShader(const ShaderFileData&);
	};

	template<>
	inline void OpenGLShader::SetUniform<bool>(u32 id , const bool value)
	{
		glUniform1i(id, value);
	}

	template<>
	inline void OpenGLShader::SetUniform<i32>(u32 id, const i32 value)
	{
		glUniform1i(id, value);
	}
	template<>
	inline void OpenGLShader::SetUniform<u32>(u32 id, const u32 value)
	{
		glUniform1ui(id, value);
	}
	template<>
	inline void OpenGLShader::SetUniform<float>(u32 id,  const float value)
	{
		glUniform1f(id, value);
	}
	template<>
	inline void OpenGLShader::SetUniformRef<V3>(u32 id, const V3& value)
	{
		glUniform3fv(id, 1, glm::value_ptr(value));
	}
	template<>
	inline void OpenGLShader::SetUniformRef<V4>(u32 id, const V4& value)
	{
		glUniform4fv(id, 1, glm::value_ptr(value));
	}
	template<>
	inline void OpenGLShader::SetUniformRef<Mat3>(u32 id, const Mat3& value)
	{
		glUniformMatrix3fv(id, 1, GL_FALSE, glm::value_ptr(value));
	}
	template<>
	inline void OpenGLShader::SetUniformRef<Mat4>(u32 id, const Mat4& value)
	{
		glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(value));
	}

}