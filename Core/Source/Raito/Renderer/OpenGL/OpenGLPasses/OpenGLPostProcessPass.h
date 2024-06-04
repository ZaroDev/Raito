#pragma once

#include <Raito/Renderer/OpenGL/OpenGLCommon.h>

#include "Renderer/OpenGL/OpenGLObjects/OpenGLFrameBuffer.h"

namespace Raito::Renderer
{
	class Camera;
}

namespace Raito::Renderer::OpenGL::PostProcess
{
	bool Initialize();
	void EnableBloom(bool value);
	void Update(const OpenGLFrameBuffer& buffer);
	void Shutdown();
}
