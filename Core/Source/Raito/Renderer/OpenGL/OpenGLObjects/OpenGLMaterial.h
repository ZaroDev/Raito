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

#include <Renderer/OpenGL/OpenGLObjects/OpenGLShader.h>
#include <utility>


namespace Raito::Renderer::OpenGL
{
	struct UniformValue
	{
		UniformValue() = default;
		UniformValue(Uniform data, ubyte* value, size_t size)
			: Data(data), Value(value), Size(size){}

		~UniformValue()
		{
			delete[] Value;
		}

		UniformValue(UniformValue const& rhs)
		{
			Size = rhs.Size;
			Data = rhs.Data;

			Value = new ubyte[Size];
			memcpy(Value, rhs.Value, Size);
		}

		UniformValue& operator=(UniformValue const& rhs)
		{
			if(this == &rhs)
			{
				return *this;
			}

			ubyte* tmp = new ubyte[rhs.Size];
			memcpy(tmp, rhs.Value, rhs.Size);

			delete[] Value;

			Value = tmp;
			Size = rhs.Size;
			Data = rhs.Data;

			return *this;
		}

		UniformValue(UniformValue&& rhs) noexcept
			: Data(rhs.Data), Value(rhs.Value), Size(rhs.Size)
		{
			rhs.Value = nullptr;
		}

		UniformValue& operator=(UniformValue&& rhs) noexcept
		{
			if(this == &rhs)
			{
				return *this;
			}

			delete[] Value;
			Value = rhs.Value;
			rhs.Value = nullptr;

			Size = rhs.Size;
			Data = rhs.Data;

			return *this;
		}

		Uniform Data{};
		ubyte* Value = nullptr;
		size_t Size = 0;

	};

	class OpenGLMaterial final
	{
	public:
		explicit OpenGLMaterial(u32 shaderId);
		~OpenGLMaterial() = default;

		OpenGLMaterial(OpenGLMaterial const& rhs)
		{
			m_ShaderId = rhs.m_ShaderId;
			m_Uniforms.clear();
			for(const auto& uniform : rhs.m_Uniforms)
			{
				m_Uniforms[uniform.first] = uniform.second;
			}
		}

		OpenGLMaterial& operator=(const OpenGLMaterial& rhs)
		{
			if (this != &rhs)
			{
				m_ShaderId = rhs.m_ShaderId; 
				m_Uniforms.clear();

				for (const auto& pair : rhs.m_Uniforms)
				{
					m_Uniforms[pair.first] = pair.second;
				}
			}
			return *this;
		}

		OpenGLMaterial(OpenGLMaterial&& rhs) noexcept
		{
			m_ShaderId = rhs.m_ShaderId;
			m_Uniforms = std::move(rhs.m_Uniforms);
			rhs.m_ShaderId = 0; // Or any other invalid value
		}

		OpenGLMaterial& operator=(OpenGLMaterial&& rhs) noexcept
		{
			if (this != &rhs)
			{
				m_ShaderId = rhs.m_ShaderId;
				m_Uniforms = std::move(rhs.m_Uniforms);
				rhs.m_ShaderId = 0; // Or any other invalid value
			}
			return *this;
		}

		void Bind();
		void UnBind() const;

		void SetValuePtr(const char* name, ubyte* value, size_t size)
		{
			if (!m_Uniforms.contains(name))
			{
				O_WARN("Material doesn't contain uniform {0}", name);
				return;
			}
			delete m_Uniforms[name].Value;

			m_Uniforms[name].Value = new ubyte[size];
			m_Uniforms[name].Size = size;
			memcpy(m_Uniforms[name].Value, value, size);
		}

		template<typename T>
		void SetValue(const char* name, const T value)
		{
			if(!m_Uniforms.contains(name))
			{
				O_WARN("Material doesn't contain uniform {0}", name);
				return;
			}
			delete m_Uniforms[name].Value;
			m_Uniforms[name].Value = new ubyte[sizeof(T)];
			m_Uniforms[name].Size = sizeof(T);
			memcpy(m_Uniforms[name].Value, &value, sizeof(T));
		}

	private:
		u32 m_ShaderId;
		std::unordered_map<std::string, UniformValue> m_Uniforms{};
	};
}
