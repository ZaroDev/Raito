#pragma once

#include <Raito/Renderer/OpenGL/OpenGLCommon.h>	


namespace Raito::Renderer::OpenGL::Deferred
{
	bool Initialize();
	void EnableParallax(bool value);
	void Update(const Camera& camera, const OpenGLFrameBuffer& buffer);

	void Shutdown();

	NODISCARD u32 GetDeferredAttachment(u32 id);
	NODISCARD u32 GetLightAttachment(u32 id);
	NODISCARD u32 GetDeferredDepth();
}
