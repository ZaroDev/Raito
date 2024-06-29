#pragma once
#include <Raito/Renderer/OpenGL/OpenGLCommon.h>

namespace Raito::Renderer::OpenGL::LightPass
{
	bool Initialize();
	void Update(const Camera& camera);
	u32 GetDirectionalSSBO();
	u32 GetPointSSBO();
	u32 GetPointSize();

	void Shutdown();
}