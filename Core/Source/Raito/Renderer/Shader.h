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
#include <filesystem>

namespace Raito::Renderer
{
	
	enum EngineShader : u32
	{
		DEFAULT_MESH = 0,
		G_BUFFER,
		DEFERRED,
		DEFERRED_POINT_LIGHT,
		DEFERRED_DIRECTIONAL_LIGHT,
		DEFAULT_LIGHT,
		SKYBOX,
		GAUSSIAN_BLUR,
		EQUIRECTANGULAR_TO_CUBEMAP,
		IRRADIANCE,
		POST_PROCESS,
		BLOOM_DOWN_SAMPLE,
		BLOOM_UP_SAMPLE,
		DEPTH,
		SSAO,
		SSAO_BLUR,
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