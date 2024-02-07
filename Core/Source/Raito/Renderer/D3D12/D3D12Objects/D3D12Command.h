#pragma once

#include <Raito/Renderer/D3D12/D3D12Common.h>

namespace Raito::Renderer::D3D12
{
	class D3D12Command
	{
	public:
		D3D12Command() = default;

		DISABLE_MOVE_AND_COPY(D3D12Command);

		explicit D3D12Command(D3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
		~D3D12Command();

		void Flush();
		void Release();

		void BeginFrame();
		void EndFrame();

		NODISCARD constexpr ID3D12CommandQueue* CommandQueue() const {}


	private:

		struct CommandFrame
		{
			ID3D12CommandAllocator* CmdAllocator = nullptr;
			u64 FenceValue = 0;

			void Wait(HANDLE fenceEvent, ID3D12Fence1* fence) const;
			void Release();
		};

		ID3D12CommandQueue* m_CommandQueue = nullptr;
		D3D12GraphicsCommandList* m_CommandList = nullptr;

		ID3D12Fence1* m_Fence = nullptr;
		u64 m_FenceValue = 0;
		HANDLE m_FenceEvent = {};

		CommandFrame m_CmdFrames[c_FrameBufferCount] = {};
		u32 m_FrameIndex = 0;
	};
}