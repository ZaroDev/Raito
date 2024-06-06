#pragma once

#include <Raito/Renderer/OpenGL/OpenGLCommon.h>

namespace Raito::Renderer::OpenGL::DeferredPlus
{
	bool Initialize();
	void Update(const Camera& camera, const OpenGLFrameBuffer& buffer);
	NODISCARD u32 GetDeferredAttachment(u32 id);
	NODISCARD u32 GetDeferredDepth();
	void Shutdown();
}