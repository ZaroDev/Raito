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
#include "OpenGLShader.h"

namespace Raito::Renderer::OpenGL
{
	OpenGLShader::OpenGLShader(u32 id) : m_ShaderId(id)
	{
		// Get uniform locations
		GLint uniformCount;
		glGetProgramiv(m_ShaderId, GL_ACTIVE_UNIFORMS, &uniformCount);

		for(GLuint i = 0; i < uniformCount; i++)
		{
			GLchar name[512];
			GLint size;
			GLenum type;
			glGetActiveUniform(m_ShaderId, i, 512, NULL, &size, &type, name);

			ShaderValue uniform;
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

		// Get attribute locations
		GLint attributeCount;
		glGetProgramiv(m_ShaderId, GL_ACTIVE_ATTRIBUTES, &attributeCount);

		for (u32 i = 0; i < attributeCount; i++)
		{
			GLchar name[512];
			GLint size;
			GLenum type;
			glGetActiveAttrib(m_ShaderId, i, 512, NULL, &size, &type, name);

			ShaderValue attribute;
			attribute.Id = glGetAttribLocation(m_ShaderId, name);

			switch (type)
			{
			case GL_INT:
				attribute.Type = UniformType::INT;
				break;
			case GL_FLOAT:
				attribute.Type = UniformType::FLOAT;
				break;
			case GL_FLOAT_VEC3:
				attribute.Type = UniformType::VEC3;
				break;
			case GL_FLOAT_MAT3:
				attribute.Type = UniformType::MAT3;
				break;
			case GL_FLOAT_MAT4:
				attribute.Type = UniformType::MAT4;
				break;
			case GL_SAMPLER_2D:
				attribute.Type = UniformType::SAMPLER_2D;
				break;

			default:
				O_WARN("Uniform {0} of type {1} can't be parsed!", name, type);
				continue;
			}

			m_Attributes[name] = attribute;
			O_LOG("Uniform {0}, registered with type {1} and id {2}", name, (u16)attribute.Type, attribute.Id);
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
