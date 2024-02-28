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

#include "pch.h"
#include "OpenGLMaterial.h"

#include <ranges>

namespace Raito::Renderer::OpenGL
{
	OpenGLMaterial::OpenGLMaterial(u32 shaderId)
		: m_ShaderId(shaderId)
	{
		const auto& shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShader(m_ShaderId));
		for (const auto& uniform : shader->Uniforms())
		{
			m_Uniforms[uniform.first] = UniformValue{ uniform.second, nullptr, 0 };
		}
	}


	void OpenGLMaterial::Bind()
	{
		const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShader(m_ShaderId));
		shader->Bind();
		for (const auto& data : m_Uniforms | std::views::values)
		{
			if(!data.Value)
			{
				continue;
			}

			switch (data.Data.Type)
			{
			case UniformType::NONE:
				break;
			case UniformType::INT:
				glUniform1i(data.Data.Id, *reinterpret_cast<GLint*>(data.Value));
				break;
			case UniformType::FLOAT:
			{
				glUniform1f(data.Data.Id, *reinterpret_cast<GLfloat*>(data.Value));
			}
			break;
			case UniformType::VEC3:
			{
				glUniform3fv(data.Data.Id, 1, reinterpret_cast<GLfloat*>(data.Value));
			}break;
			case UniformType::VEC4:
			{
				glUniform4fv(data.Data.Id, 1, reinterpret_cast<GLfloat*>(data.Value));
			}
			break;
			case UniformType::MAT3:
			{
				glUniformMatrix3fv(data.Data.Id, 1, GL_FALSE, reinterpret_cast<GLfloat*>(data.Value));
			}break;
			case UniformType::MAT4:
			{
				glUniformMatrix4fv(data.Data.Id, 1, GL_FALSE, reinterpret_cast<GLfloat*>(data.Value));
			}
			break;
			case UniformType::SAMPLER_2D:
			{
				const TextureData textureData = *reinterpret_cast<TextureData*>(data.Value);
				glUniformHandleui64ARB(data.Data.Id, textureData.Handle);

			}
			break;
			}
		}
	}

	void OpenGLMaterial::UnBind() const
	{
		const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShader(m_ShaderId));
		shader->UnBind();
	}
}
