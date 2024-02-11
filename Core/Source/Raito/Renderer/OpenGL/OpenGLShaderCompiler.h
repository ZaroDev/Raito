#pragma once

#include "OpenGLCommon.h"

namespace Raito::Renderer::OpenGL
{
	enum OpenGLEngineShader : u32;

	class OpenGLShader;
}

namespace Raito::Renderer::OpenGL::ShaderCompiler
{
	struct ShaderFileData
	{
		const char* FilePath;
		OpenGLEngineShader Id;
		int Type;
	};

	bool Initialize();

	u32 CompileShader(const ShaderFileData& data);

	OpenGLShader* GetShader(u32 id);
	OpenGLShader* GetShader(OpenGLEngineShader id);


	void Shutdown();
}
