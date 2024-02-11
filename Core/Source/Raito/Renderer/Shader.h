#pragma once
#include <filesystem>

namespace Raito::Renderer
{
	
	enum EngineShader : u32
	{
		UNSHADED_MESH = 0,
		POST_PROCESS,
		ENGINE_SHADER_MAX
	};

	struct ShaderFileData
	{
		const char* FilePath;
		EngineShader Id;
		int Type;
	};


	class Shader
	{
	public:

		Shader(){}
		virtual ~Shader(){}

		NODISCARD EngineShader EngineType() const { return m_EngineType; }
		NODISCARD std::string FilePath() const { return m_FilePath.string(); }
	protected:
		std::filesystem::path m_FilePath;
		EngineShader m_EngineType;
	};
}