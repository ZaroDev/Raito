#include <pch.h>
#include "OpenGLRHI.h"

#include "Renderer/RHI.h"
#include "OpenGLCore.h"


namespace Raito::Renderer::OpenGL
{
	void GetPlatformInterface(RHI& rhi)
	{
		rhi.Initialize = Initialize;
		rhi.Shutdown = Shutdown;
	}
}
