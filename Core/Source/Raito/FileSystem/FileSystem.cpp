#include <pch.h>
#include "FileSystem/FileSystem.h"

#include "Renderer/Renderer.h"

namespace Raito::FileSystem
{
	bool ReadFile(const std::filesystem::path& path, std::shared_ptr<uint8_t[]>& data, uint64_t& size)
	{
		if (!std::filesystem::exists(path))
			return false;

		size = std::filesystem::file_size(path);
		ASSERT(size);
		data = std::make_shared<u8[]>(size);
		std::ifstream file(path, std::ios::in | std::ios::binary);
		if (!file || !file.read((char*)data.get(), size))
		{
			file.close();
			return false;
		}

		file.close();
		return true;
	}
	bool LoadEngineShaders(std::shared_ptr<uint8_t[]>& shaders, uint64_t& size)
	{
		auto path = Renderer::GetEngineShadersPath(Renderer::API::D3D12);
		return ReadFile(path, shaders, size);
	}
}
