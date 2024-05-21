#pragma once

#include "D3D12Common.h"
#include "Assets/Mesh.h"
#include "Assets/Texture.h"


namespace Raito::Renderer::D3D12::Upload
{
	u32 AddMesh(Assets::Mesh* mesh);
	u32 AddMaterial(EngineShader shader);
	u32 AddTexture(Assets::Texture* texture, ubyte* data);

	void SetMaterialValue(u32 id, const char* name, ubyte* data, size_t size);
	nvrhi::BufferHandle GetMesh(u32 id);
}
