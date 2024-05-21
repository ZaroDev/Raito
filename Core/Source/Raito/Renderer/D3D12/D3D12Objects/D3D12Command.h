#pragma once
#include <Raito/Renderer/D3D12/D3D12Common.h>

#include "D3D12Surface.h"

namespace Raito::Renderer::D3D12
{

    class D3D12Command final
	{
	public:
		D3D12Command() = default;
		DISABLE_COPY(D3D12Command)
		DISABLE_MOVE(D3D12Command)


		explicit D3D12Command(D3D12Device* const device, D3D12_COMMAND_LIST_TYPE type);
        ~D3D12Command();

        void CreateCommandList();
        void Flush();
        void Release();

        // Wait for the current frame to be signaled and reset the command list/allocator.
        void BeginFrame();
        // Signal the fence with the new fence value.
        void EndFrame(const D3D12Surface& surface);
        
        NODISCARD constexpr ID3D12CommandQueue* CommandQueue() const { return m_CommandQueue; }
        NODISCARD nvrhi::CommandListHandle CommandList() const { return m_CommandList; }
        NODISCARD constexpr u32 FrameIndex() const { return m_FrameIndex; }

    private:
        struct CommandFrame
        {
            ID3D12CommandAllocator* CmdAllocator = nullptr;
            uint64_t FenceValue = 0;

            void Wait(HANDLE fenceEvent, ID3D12Fence1* fence) const;
            void Release();
        };


        ID3D12CommandQueue* m_CommandQueue = nullptr;
        nvrhi::CommandQueue m_CommandType; 
        nvrhi::CommandListHandle m_CommandList = nullptr;

        ID3D12Fence1* m_Fence = nullptr;
        uint64_t m_FenceValue = 0;
        HANDLE m_FenceEvent = {};

        CommandFrame m_CmdFrames[c_FrameBufferCount] = {};
        u32 m_FrameIndex = 0;
	};
}
