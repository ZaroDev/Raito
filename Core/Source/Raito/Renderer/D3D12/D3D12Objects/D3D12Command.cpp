#include "pch.h"
#include "D3D12Command.h"

namespace Raito::Renderer::D3D12
{
	D3D12Command::D3D12Command(D3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
	{
		HRESULT hr = S_OK;

		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Type = type;
		DXCall(hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_CommandQueue)));

		if(FAILED(hr))
		{
			Release();
		}

		NAME_D3D12_OBJECT(m_CommandQueue,
			type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"GFX Command Queue"
			: type == D3D12_COMMAND_LIST_TYPE_COMPUTE ? L"Compute Command Queue" : L"Command queue");

		for(u32 i = 0; i < c_FrameBufferCount; i++)
		{
			
		}
	}

	D3D12Command::~D3D12Command()
	{
	}

	void D3D12Command::Flush()
	{
	}

	void D3D12Command::Release()
	{
	}

	void D3D12Command::BeginFrame()
	{
	}

	void D3D12Command::EndFrame()
	{
	}

	void D3D12Command::CommandFrame::Wait(HANDLE fenceEvent, ID3D12Fence1* fence) const
	{
	}

	void D3D12Command::CommandFrame::Release()
	{
	}
}
