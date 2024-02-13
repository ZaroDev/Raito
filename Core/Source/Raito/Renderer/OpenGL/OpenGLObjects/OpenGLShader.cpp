#include <pch.h>
#include "OpenGLShader.h"

namespace Raito::Renderer::OpenGL
{
	OpenGLShader::OpenGLShader(u32 id) : m_ShaderId(id)
	{
		GLint uniformCount;

		glGetProgramiv(m_ShaderId, GL_ACTIVE_UNIFORMS, &uniformCount);

		for(GLuint i = 0; i < uniformCount; i++)
		{
			GLchar name[512];
			GLint size;
			GLenum type;
			glGetActiveUniform(m_ShaderId, i, 512, NULL, &size, &type, name);

			Uniform uniform;
			uniform.Id = glGetUniformLocation(m_ShaderId, name);
			
			switch (type)
			{
			case GL_INT: 
				uniform.Type = UniformType::INT;
				break;
			case GL_FLOAT:
				uniform.Type = UniformType::FLOAT;
				break;
			case GL_FLOAT_VEC3:
				uniform.Type = UniformType::VEC3;
				break;
			case GL_FLOAT_MAT3:
				uniform.Type = UniformType::MAT3;
				break;
			case GL_FLOAT_MAT4:
				uniform.Type = UniformType::MAT4;
				break;
			case GL_SAMPLER_2D:
				uniform.Type = UniformType::SAMPLER_2D;
				break;

			default:
				O_WARN("Uniform {0} of type {1} can't be parsed!", name, type);
				continue;
			}

			m_Uniforms[name] = uniform;

			O_LOG("Uniform {0}, registered with type {1} and id {2}", name, (u16)uniform.Type, uniform.Id);
		}
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_ShaderId);
		m_Uniforms.clear();
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_ShaderId);
	}

	void OpenGLShader::UnBind() const
	{
		glUseProgram(0);
	}
}
