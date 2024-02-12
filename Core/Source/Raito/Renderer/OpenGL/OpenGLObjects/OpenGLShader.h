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
		void SetUniform(const char* uniformName, const T value)
		{
			O_ERROR("Uniform specialization not found for {0}", uniformName);
			ASSERT(false);
		}
		template<typename T>
		void SetUniformRef(const char* uniformName, const T& value)
		{
			O_ERROR("Uniform specialization not found for {0}", uniformName);
			ASSERT(false);
		}

	
		NODISCARD OpenGLShaderType ShaderType() const { return m_ShaderType; }
		NODISCARD size_t UniformCount() const { return m_Uniforms.size(); }
	
	private:
		GLint GetUniformLocation(const char* uniformName) const
		{
			return m_Uniforms.at(uniformName).Id;
		}

	private:
		u32 m_ShaderId = 0;
		OpenGLShaderType m_ShaderType = OpenGLShaderType::MAX;

		std::unordered_map<std::string, Uniform> m_Uniforms{};

		friend u32 ShaderCompiler::CompileShader(const ShaderFileData&);
	};


	template<>
	inline void OpenGLShader::SetUniform<i32>(const char* uniformName, const i32 value)
	{
		glUniform1i(GetUniformLocation(uniformName), value);
	}
	template<>
	inline void OpenGLShader::SetUniform<u32>(const char* uniformName, const u32 value)
	{
		glUniform1ui(GetUniformLocation(uniformName), value);
	}
	template<>
	inline void OpenGLShader::SetUniform<float>(const char* uniformName, const float value)
	{
		glUniform1f(GetUniformLocation(uniformName), value);
	}
	template<>
	inline void OpenGLShader::SetUniformRef<V3>(const char* uniformName, const V3& value)
	{
		glUniform3fv(GetUniformLocation(uniformName), 1, glm::value_ptr(value));
	}
	template<>
	inline void OpenGLShader::SetUniformRef<V4>(const char* uniformName, const V4& value)
	{
		glUniform4fv(GetUniformLocation(uniformName), 1, glm::value_ptr(value));
	}
	template<>
	inline void OpenGLShader::SetUniformRef<Mat3>(const char* uniformName, const Mat3& value)
	{
		glUniformMatrix3fv(GetUniformLocation(uniformName), 1, GL_FALSE, glm::value_ptr(value));
	}
	template<>
	inline void OpenGLShader::SetUniformRef<Mat4>(const char* uniformName, const Mat4& value)
	{
		glUniformMatrix4fv(GetUniformLocation(uniformName), 1, GL_FALSE, glm::value_ptr(value));
	}

}