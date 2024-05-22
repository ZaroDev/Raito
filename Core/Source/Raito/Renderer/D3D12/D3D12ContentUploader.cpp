#include <pch.h>
#include "D3D12ContentUploader.h"

#include <Raito/Renderer/D3D12/D3D12Core.h>
#include "D3D12Objects/D3D12Command.h"

namespace Raito::Renderer::D3D12::Upload
{
	namespace
	{
		std::vector<D3D12MeshData> g_Meshes{};
		std::vector<nvrhi::TextureHandle> g_Textures{};
	}

	u32 AddMesh(Assets::Mesh* mesh)
	{
		const auto vertexBufferDesc = nvrhi::BufferDesc()
			.setByteSize(mesh->Vertices.size() * sizeof(Assets::Vertex))
			.setIsVertexBuffer(true)
			.setInitialState(nvrhi::ResourceStates::VertexBuffer)
			.setKeepInitialState(true)
			.setDebugName("Vertex buffer");
		const auto vertexBuffer = Core::NVDevice()->createBuffer(vertexBufferDesc);
		const auto vertexBufferBinding = nvrhi::VertexBufferBinding()
			.setBuffer(vertexBuffer)
			.setSlot(0)
			.setOffset(0);

		const auto indexBufferDesc = nvrhi::BufferDesc()
			.setByteSize(mesh->Indices.size() * sizeof(u32))
			.setInitialState(nvrhi::ResourceStates::IndexBuffer)
			.setIsIndexBuffer(true)
			.setKeepInitialState(true)
			.setDebugName("Index buffer");

		const auto indexBuffer = Core::NVDevice()->createBuffer(indexBufferDesc);
		const auto indexBufferBinding = nvrhi::IndexBufferBinding()
			.setBuffer(indexBuffer)
			.setFormat(nvrhi::Format::R32_UINT);

		const auto& meshData = g_Meshes.emplace_back(D3D12MeshData{ 
			vertexBuffer, vertexBufferBinding,
			indexBuffer, indexBufferBinding,static_cast<u32>(mesh->Indices.size())
		});

		const nvrhi::CommandListHandle commandList = Core::GraphicsCommand().CommandList();
		commandList->open();
		commandList->writeBuffer(meshData.VertexBuffer, mesh->Vertices.data(), mesh->Vertices.size() * sizeof(Assets::Vertex));
		commandList->writeBuffer(meshData.IndexBuffer, mesh->Indices.data(), mesh->Indices.size() * sizeof(u32));
		commandList->close();

		Core::NVDevice()->executeCommandList(commandList);
		return g_Meshes.size() - 1;
	}

	u32 AddMaterial(EngineShader shader)
	{
		return 0;
	}

	u32 AddTexture(Assets::Texture* texture, ubyte* data)
	{
		auto textureDesc = nvrhi::TextureDesc()
			.setDimension(nvrhi::TextureDimension::Texture2D)
			.setWidth(texture->Width)
			.setHeight(texture->Height)
			.setFormat(nvrhi::Format::RGBA8_UNORM)
			.setInitialState(nvrhi::ResourceStates::ShaderResource)
			.setKeepInitialState(true)
			.setDebugName("Texture");

		const auto& textureH = g_Textures.emplace_back(Core::NVDevice()->createTexture(textureDesc));

        const size_t rowPitch = texture->Width * 4;
		const nvrhi::CommandListHandle& commandList = Core::GraphicsCommand().CommandList();
		commandList->open();
		commandList->writeTexture(textureH, 0, 0, data, rowPitch);
		commandList->close();
		Core::NVDevice()->executeCommandList(commandList);

		return g_Textures.size() - 1;
	}

	void SetMaterialValue(u32 id, const char* name, ubyte* data, size_t size)
	{
	}

	D3D12MeshData GetMesh(u32 id)
	{
		return g_Meshes[id];
	}

	nvrhi::TextureHandle GetTexture(u32 id)
	{
		return g_Textures[id];
	}
}


