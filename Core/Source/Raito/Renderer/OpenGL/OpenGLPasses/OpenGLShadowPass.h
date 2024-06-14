#pragma once

#include <Raito/Renderer/OpenGL/OpenGLCommon.h>

namespace Raito::Renderer::OpenGL::Shadows
{

	

	bool Initialize();
	void Enable(bool value);
	void Update(const Camera& camera);
	void Shutdown();
	u32 GetShadowMapSSBO();
	u32 GetLightMatricesSSBO();
	u32 GetShadowMap();
	const std::vector<float>& GetCascadeLevels();
}