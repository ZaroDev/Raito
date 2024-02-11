#pragma once

#include <Renderer/OpenGL/OpenGLCommon.h>
#include <Renderer/OpenGL/OpenGLShaderCompiler.h>

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


	enum OpenGLEngineShader : u32
	{
		UNSHADED_MESH = 0,
		POST_PROCESS,
		ENGINE_SHADER_MAX
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

	class OpenGLShader final
	{
	public:
		OpenGLShader(u32 id);
		~OpenGLShader();


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
		void SetUniform(const char* uniformName, const T& value)
		{
			O_ERROR("Uniform specialization not found for {0}", uniformName);
			ASSERT(false);
		}



	private:
		GLint GetUniformLocation(const char* uniformName) const
		{
			return m_Uniforms.at(uniformName).Id;
		}

	private:
		std::filesystem::path m_FilePath{};
		u32 m_ShaderId = 0;

		OpenGLShaderType m_ShaderType = OpenGLShaderType::MAX;

		std::unordered_map<std::string, Uniform> m_Uniforms{};

		friend u32 ShaderCompiler::CompileShader(const ShaderCompiler::ShaderFileData&);
	};


	template<>
	inline void OpenGLShader::SetUniform(const char* uniformName, const i32 value)
	{
		glUniform1i(GetUniformLocation(uniformName), value);
	}
	template<>
	inline void OpenGLShader::SetUniform(const char* uniformName, const u32 value)
	{
		glUniform1ui(GetUniformLocation(uniformName), value);
	}
	template<>
	inline void OpenGLShader::SetUniform(const char* uniformName, const float value)
	{
		glUniform1f(GetUniformLocation(uniformName), value);
	}
	template<>
	inline void OpenGLShader::SetUniform<V3>(const char* uniformName, const V3& value)
	{
		glUniform3fv(GetUniformLocation(uniformName), 1, glm::value_ptr(value));
	}
	template<>
	inline void OpenGLShader::SetUniform<V4>(const char* uniformName, const V4& value)
	{
		glUniform4fv(GetUniformLocation(uniformName), 1, glm::value_ptr(value));
	}
	template<>
	inline void OpenGLShader::SetUniform<Mat3>(const char* uniformName, const Mat3& value)
	{
		glUniformMatrix3fv(GetUniformLocation(uniformName), 1, GL_FALSE, glm::value_ptr(value));
	}
	template<>
	inline void OpenGLShader::SetUniform<Mat4>(const char* uniformName, const Mat4& value)
	{
		glUniformMatrix4fv(GetUniformLocation(uniformName), 1, GL_FALSE, glm::value_ptr(value));
	}

}