#pragma once
#include <Raito/Renderer/OpenGL/OpenGLCommon.h>

#include "Renderer/OpenGL/OpenGLObjects/OpenGLFrameBuffer.h"

namespace Raito::Renderer
{
	class Camera;
}

namespace Raito::Renderer::OpenGL::SSAO
{
	bool Initialize();
	void Update(Camera* camera, const OpenGLFrameBuffer& buffer);
	NODISCARD u64 GetSSAOHandle();
	NODISCARD u32 GetSSAOAttachment();
	void Shutdown();
}
