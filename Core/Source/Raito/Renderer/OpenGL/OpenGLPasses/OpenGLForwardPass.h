#pragma once
#include <Raito/Renderer/OpenGL/OpenGLCommon.h>


namespace Raito::Renderer::OpenGL::Forward
{
	bool Initialize();
	void Update(const Camera& camera, const OpenGLFrameBuffer& buffer);
	void Shutdown();
}
