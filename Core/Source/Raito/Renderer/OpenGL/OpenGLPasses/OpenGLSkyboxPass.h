#pragma once
#include "Renderer/Camera.h"

namespace Raito::Renderer::OpenGL::Skybox
{
	bool Initialize();
	void Update(Camera* camera);
	void Shutdown();
	u32 GetIrradianceMap();
}
