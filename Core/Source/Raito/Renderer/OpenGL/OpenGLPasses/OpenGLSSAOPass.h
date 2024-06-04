#pragma once
#include <Raito/Renderer/OpenGL/OpenGLCommon.h>

namespace Raito::Renderer::OpenGL::SSAO
{
	bool Initialize();
	void Enable(bool value);

	void Update(const Camera& camera, const OpenGLFrameBuffer& buffer);
	NODISCARD u64 GetSSAOHandle();
	NODISCARD u32 GetSSAOAttachment();
	void Shutdown();
}
