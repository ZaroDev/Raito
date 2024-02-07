#pragma once

#include <Raito/Renderer/D3D12/D3D12Common.h>

namespace Raito::Renderer::D3D12
{
	struct DescriptorHandle
	{
		D3D12_CPU_DESCRIPTOR_HANDLE Cpu = {};
		D3D12_GPU_DESCRIPTOR_HANDLE Gpu = {};

		u32 Index = U32_MAX;
		NODISCARD constexpr bool IsValid() const { return Cpu.ptr != 0; }
		NODISCARD constexpr bool IsShaderVisible() const { return Gpu.ptr != 0; }
	};
}
