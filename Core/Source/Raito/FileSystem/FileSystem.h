#pragma once

namespace Raito::FileSystem
{
	bool ReadFile(const std::filesystem::path& path, std::shared_ptr<uint8_t[]>& data, uint64_t& size);
	bool LoadEngineShaders(std::shared_ptr<uint8_t[]>& shaders, uint64_t& size);
}