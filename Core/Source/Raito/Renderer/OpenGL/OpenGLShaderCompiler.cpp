#include <pch.h>
#include "OpenGLShaderCompiler.h"

#include <Renderer/OpenGL/OpenGLObjects/OpenGLShader.h>

namespace Raito::Renderer::OpenGL::ShaderCompiler
{
	namespace 
	{
		std::vector<OpenGLShader> g_Shaders{};

		

		constexpr const char* c_ShadersSourceFiles = "Shaders";

		// NOTE: This must be in the same order as OpenGLEngineShader to have the ID's be the same as the enum values
		constexpr ShaderFileData c_ShaderFiles[]
		{
			{"UnshadedMesh", OpenGLEngineShader::UNSHADED_MESH, OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT },
			{ "PostProcess", OpenGLEngineShader::POST_PROCESS,  OpenGLShaderType::VERTEX | OpenGLShaderType::FRAGMENT },
		};

		static_assert(_countof(c_ShaderFiles) == OpenGLEngineShader::ENGINE_SHADER_MAX);

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
				O_ERROR("Vertex shader not succesfully readed!");
			}

			const char* cCode = code.c_str();

			const u32 shader = glCreateShader(type);

			glShaderSource(shader, 1, &cCode, NULL);
			glCompileShader(shader);

			GLint success;

			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

			if(success == GL_FALSE)
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
			CompileShader(shaderFile);
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

		if(data.Type & OpenGLShaderType::FRAGMENT)
		{
			filePath.replace_extension(".frag");

			const u32 fragment = CreateShader(filePath, GL_FRAGMENT_SHADER);

			glAttachShader(programId, fragment);
			glDeleteShader(fragment);
		}

		if(data.Type & OpenGLShaderType::GEOMETRY)
		{
			filePath.replace_extension(".geo");

			const u32 geometry = CreateShader(filePath, GL_FRAGMENT_SHADER);

			glAttachShader(programId, geometry);
			glDeleteShader(geometry);
		}

		if(data.Type & OpenGLShaderType::COMPUTE)
		{
			filePath.replace_extension(".geo");

			const u32 compute = CreateShader(filePath, GL_FRAGMENT_SHADER);

			glAttachShader(programId, compute);
			glDeleteShader(compute);
		}

		glLinkProgram(programId);

		GLint success;
		glGetProgramiv(programId, GL_LINK_STATUS, &success);

		if(success == GL_FALSE)
		{
			char infoLog[512];
			glGetProgramInfoLog(programId, 512, NULL, infoLog);
			O_ERROR("Shader linking failed {0}", infoLog);
		}


		filePath.replace_extension("");


		OpenGLShader& shader = g_Shaders.emplace_back(programId);
		shader.m_ShaderType = static_cast<OpenGLShaderType>(data.Type);
		shader.m_FilePath = filePath;


		return (u32)g_Shaders.size() - 1;
	}

	OpenGLShader* GetShader(u32 id)
	{
		return &g_Shaders[id];
	}

	OpenGLShader* GetShader(OpenGLEngineShader id)
	{
		return &g_Shaders[id];
	}

	void Shutdown()
	{
		g_Shaders.clear();
	}
}
