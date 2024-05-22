#pragma once

#include "D3D12Common.h"
#include "Assets/Mesh.h"
#include "Assets/Texture.h"


namespace Raito::Renderer::D3D12::Upload
{
	struct D3D12MeshData
	{
		nvrhi::BufferHandle			VertexBuffer;
		nvrhi::VertexBufferBinding	VertexBufferBinding;
		nvrhi::BufferHandle			IndexBuffer;
		nvrhi::IndexBufferBinding	IndexBufferBinding;
		u32						IndexCount;
	};

	u32 AddMesh(Assets::Mesh* mesh);
	u32 AddMaterial(EngineShader shader);
	u32 AddTexture(Assets::Texture* texture, ubyte* data);

	void SetMaterialValue(u32 id, const char* name, ubyte* data, size_t size);
	D3D12MeshData GetMesh(u32 id);
	nvrhi::TextureHandle GetTexture(u32 id);

}
