#pragma once
#include <Raito/Renderer/OpenGL/OpenGLCommon.h>

namespace Raito::Renderer::OpenGL::Skybox
{
	bool Initialize();
	void Update(const Camera& camera);
	void Shutdown();
	u32 GetIrradianceMap();
	u32 GetPrefilterMap();
	u64 GetBRDFLUTTMap();
}
