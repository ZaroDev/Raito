#pragma once
#include <Raito/Renderer/D3D12/D3D12Common.h>
#include <Raito/Renderer/D3D12/D3D12Objects/D3D12Surface.h>
#include "Renderer/Camera.h"


namespace Raito::Renderer::D3D12::Deferred
{
	bool Initialize();

	void Update(Renderer::Camera* camera, D3D12Surface& surface, u32 frameIndex);
	void Shutdown();
}
