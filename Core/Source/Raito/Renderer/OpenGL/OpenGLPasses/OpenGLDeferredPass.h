#pragma once

#include <Raito/Renderer/OpenGL/OpenGLCommon.h>	

#include "Renderer/OpenGL/OpenGLObjects/OpenGLFrameBuffer.h"


namespace Raito::Renderer
{
	class Camera;
}

namespace Raito::Renderer::OpenGL::Deferred
{
	bool Initialize();
	void Update(Camera* camera, const OpenGLFrameBuffer& buffer);

	void Shutdown();

	NODISCARD u32 GetDeferredAttachment(u32 id);
	NODISCARD u32 GetDeferredDepth();
}
