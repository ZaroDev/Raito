#pragma once

#include "OpenGLCommon.h"

namespace Raito::Renderer::OpenGL::ShaderCompiler
{
	bool Initialize();

	u32 CompileShader(const ShaderFileData& data);

	Shader* GetShader(u32 id);
	Shader* GetShaderWithEngineId(EngineShader id);
	ShaderFileData GetFileData(EngineShader id);

	const std::vector<Shader*>& GetAllShaders();


	void Shutdown();
}
