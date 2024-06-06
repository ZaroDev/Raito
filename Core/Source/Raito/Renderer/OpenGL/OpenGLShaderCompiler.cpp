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
#include "OpenGLShaderCompiler.h"

#include <Renderer/OpenGL/OpenGLObjects/OpenGLShader.h>

namespace Raito::Renderer::OpenGL::ShaderCompiler
{
	namespace
	{
		std::vector<Shader*> g_Shaders{};

		constexpr const char* c_ShadersSourceFiles = "Shaders/OpenGL";

		// NOTE: This must be in the same order as EngineShader to have the ID's be the same as the enum values
		constexpr ShaderFileData c_ShaderFiles[]
		{
			{"Mesh/DefaultMesh", EngineShader::DEFAULT_MESH, OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT },
			{"Deferred/GBuffer", EngineShader::G_BUFFER,  OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT},
			{"Deferred/DeferredCombine", EngineShader::DEFERRED,  OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT},
			{"Deferred/DeferredPointLight", EngineShader::DEFERRED_POINT_LIGHT,  OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT},
			{"Deferred/DeferredDirectionalLight", EngineShader::DEFERRED_DIRECTIONAL_LIGHT,  OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT},
			{"DefaultLight", EngineShader::DEFAULT_LIGHT, OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT},
			{"Skybox", EngineShader::SKYBOX, OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT},
			{"PostProcessing/GaussianBlur", EngineShader::GAUSSIAN_BLUR, OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT },
			{"EquirectangularToCubemap", EngineShader::EQUIRECTANGULAR_TO_CUBEMAP, OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT },
			{"Irradiance", EngineShader::IRRADIANCE, OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT },
			{ "PostProcessing/PostProcess", EngineShader::POST_PROCESS,  OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT },
			{ "PostProcessing/BloomDownSample", EngineShader::BLOOM_DOWN_SAMPLE,  OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT },
			{ "PostProcessing/BloomUpSample", EngineShader::BLOOM_UP_SAMPLE,  OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT },
			{ "Shadows/Depth", EngineShader::DEPTH,  OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT | OpenGLShaderType::GEOMETRY },
			{ "Deferred/SSAO", EngineShader::SSAO,  OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT  },
			{ "Deferred/SSAOBlur", EngineShader::SSAO_BLUR,  OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT },
			{ "DeferredPlus/DownSampleAndReproject", EngineShader::DOWN_SAMPLE_REPROJECT, OpenGLShaderType::COMPUTE },
		};

		static_assert(_countof(c_ShaderFiles) == EngineShader::ENGINE_SHADER_MAX);

		u32 CreateShader(const std::filesystem::path& path, GLenum type)
		{
			std::string code;
			std::ifstream file;
			file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

			try
			{
				file.open(path);

				std::stringstream stream;
				stream << file.rdbuf();

				file.close();

				code = stream.str();
			}
			catch (std::ifstream::failure e)
			{
				O_ERROR("Shader file not succesfully readed!");
			}

			const char* cCode = code.c_str();

			const u32 shader = glCreateShader(type);

			glShaderSource(shader, 1, &cCode, NULL);
			glCompileShader(shader);

			GLint success;

			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

			if (success == GL_FALSE)
			{
				char infoLog[512];
				glGetShaderInfoLog(shader, 512, NULL, infoLog);
				O_ERROR("Shader compilation failed {0}", infoLog);
			}

			return shader;
		}
	}


	bool Initialize()
	{
		for (const auto& shaderFile : c_ShaderFiles)
		{
			ShaderCompiler::CompileShader(shaderFile);
		}

		return true;
	}

	u32 CompileShader(const ShaderFileData& data)
	{
		std::filesystem::path filePath = c_ShadersSourceFiles;
		filePath /= data.FilePath;

		const u32 programId = glCreateProgram();

		if (data.Type & OpenGLShaderType::VERTEX)
		{
			filePath.replace_extension(".vert");

			const u32 vertex = CreateShader(filePath, GL_VERTEX_SHADER);

			glAttachShader(programId, vertex);
			glDeleteShader(vertex);
		}

		if (data.Type & OpenGLShaderType::FRAGMENT)
		{
			filePath.replace_extension(".frag");

			const u32 fragment = CreateShader(filePath, GL_FRAGMENT_SHADER);

			glAttachShader(programId, fragment);
			glDeleteShader(fragment);
		}

		if (data.Type & OpenGLShaderType::GEOMETRY)
		{
			filePath.replace_extension(".geo");

			const u32 geometry = CreateShader(filePath, GL_GEOMETRY_SHADER);

			glAttachShader(programId, geometry);
			glDeleteShader(geometry);
		}

		if (data.Type & OpenGLShaderType::COMPUTE)
		{
			filePath.replace_extension(".comp");

			const u32 compute = CreateShader(filePath, GL_COMPUTE_SHADER);

			glAttachShader(programId, compute);
			glDeleteShader(compute);
		}

		glLinkProgram(programId);

		GLint success;
		glGetProgramiv(programId, GL_LINK_STATUS, &success);

		if (success == GL_FALSE)
		{
			char infoLog[512];
			glGetProgramInfoLog(programId, 512, NULL, infoLog);
			O_ERROR("Shader linking failed {0}", infoLog);
		}


		filePath.replace_extension("");


		OpenGLShader* shader = new OpenGLShader(programId);
		shader->m_ShaderType = static_cast<OpenGLShaderType>(data.Type);
		shader->m_FilePath = filePath;
		shader->m_EngineType = data.Id;

		if (g_Shaders.size() > data.Id && g_Shaders.at(data.Id) != nullptr)
		{
			delete g_Shaders.at(data.Id);
			g_Shaders.erase(g_Shaders.begin() + data.Id);
			g_Shaders.insert(g_Shaders.begin() + data.Id, shader);
		}
		else
		{
			g_Shaders.emplace_back(shader);
		}

		return data.Id;
	}

	Shader* GetShader(u32 id)
	{
		return g_Shaders[id];
	}

	Shader* GetShaderWithEngineId(EngineShader id)
	{
		return g_Shaders[id];
	}

	ShaderFileData GetFileData(EngineShader id)
	{
		if (id >= EngineShader::ENGINE_SHADER_MAX)
		{
			return ShaderFileData();
		}

		return c_ShaderFiles[id];
	}

	const std::vector<Shader*>& GetAllShaders()
	{
		return g_Shaders;
	}

	void Shutdown()
	{
		for (auto& shader : g_Shaders)
		{
			delete shader;
			shader = nullptr;
		}

		g_Shaders.clear();
	}
}
