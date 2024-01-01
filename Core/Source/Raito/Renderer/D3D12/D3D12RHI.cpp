#include "pch.h"
#include "D3D12RHI.h"

#include "Renderer/RHI.h"
#include "D3D12Core.h"

namespace Raito::Renderer::D3D12
{
	void GetPlatformInterface(RHI& rhi)
	{
		rhi.Initialize = Core::Initialize;
		rhi.Shutdown = Core::Shutdown;
	}
}